import subprocess
import threading
import json
import os
import re
from flask import Flask, jsonify, request, send_from_directory

app = Flask(__name__, static_folder="web")

SIM_PATH = os.path.join(os.path.dirname(__file__), "simulador.exe")

_proc = None
_lock = threading.Lock()


def get_proc():
    global _proc
    if _proc is None or _proc.poll() is not None:
        _proc = subprocess.Popen(
            [SIM_PATH],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
        )
    return _proc


def send_cmd(cmd: str) -> str:
    proc = get_proc()
    proc.stdin.write(cmd + "\n")
    proc.stdin.flush()

    lines = []
    while True:
        line = proc.stdout.readline()
        if not line:
            break
        if line.strip() == "END":
            break
        lines.append(line)
    return "".join(lines)


def parse_response(raw: str):
    """
    A resposta pode ser:
      1) Um objeto JSON puro  -> { ... }
      2) Um objeto JSON seguido de texto de resumo (print_final_summary)
    Retorna (json_obj, summary_text)
    """
    raw = raw.strip()
    if not raw:
        return None, ""

    # Tenta encontrar o primeiro objeto JSON balanceado
    depth = 0
    end = -1
    for i, ch in enumerate(raw):
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                end = i
                break

    if end == -1:
        return None, raw

    json_str = raw[: end + 1]
    rest = raw[end + 1 :].strip()

    try:
        obj = json.loads(json_str)
    except json.JSONDecodeError:
        return None, raw

    return obj, rest


@app.route("/")
def index():
    return send_from_directory("web", "index.html")


@app.route("/api/next", methods=["POST"])
def api_next():
    with _lock:
        raw = send_cmd("next")
    obj, summary = parse_response(raw)
    return jsonify({"data": obj, "summary": summary})


@app.route("/api/run_all", methods=["POST"])
def api_run_all():
    with _lock:
        raw = send_cmd("run_all")
    _, summary = parse_response(raw)
    # run_all não retorna JSON por ciclo, lê o trace file
    trace_path = os.path.join(os.path.dirname(__file__), "pipeline_trace.json")
    trace = None
    if os.path.exists(trace_path):
        with open(trace_path) as f:
            trace = json.load(f)
    return jsonify({"trace": trace, "summary": summary})


@app.route("/api/reset", methods=["POST"])
def api_reset():
    with _lock:
        raw = send_cmd("reset")
    obj, _ = parse_response(raw)
    return jsonify({"data": obj})


if __name__ == "__main__":
    print("Abrindo simulador em http://localhost:5000")
    app.run(debug=False, port=5000)
