# Arquitetura-de-Computadores

Repositório da disciplina de Arquitetura de Computadores — UFMA 2026.1.

# 🧠 Simulador de Pipeline MIPS-Lite

Simulador educacional de arquitetura MIPS desenvolvido em linguagem C, com foco na implementação de um pipeline clássico de 5 estágios.

---

# 📚 Objetivo

O projeto tem como finalidade simular o funcionamento interno de um processador MIPS simplificado, permitindo visualizar:

- Memória de instruções
- Memória de dados
- Banco de registradores
- Program Counter (PC)
- Registradores de pipeline
- Execução por estágios
- Controle de hazards e stalls

O simulador foi desenvolvido de forma modular, separando estrutura de dados, memória e lógica do pipeline.

---

# 🏗️ Estrutura do Projeto

```txt
arquitetura-de-computadores/
│
├── main.c
├── memory.c
├── memory.h
├── pipeline.c
├── pipeline.h
├── types.h
├── README.md
└── .gitignore
```

---

# ⚙️ Componentes do Projeto

## 📄 `types.h`

Arquivo responsável por centralizar:

- Constantes globais
- Definições de memória
- Quantidade de registradores
- Structs dos registradores de pipeline
- Variáveis globais compartilhadas

### Registradores de pipeline implementados

- IF/ID
- ID/EX
- EX/MEM
- MEM/WB

---

## 📄 `memory.c`

Responsável pelo gerenciamento do estado persistente do simulador:

- Inicialização das memórias
- Inicialização dos registradores
- Inicialização do Program Counter
- Carregamento do programa na memória de instruções

---

## 📄 `memory.h`

Header contendo as funções públicas do módulo de memória.

---

## 📄 `pipeline.c`

Responsável pela implementação dos registradores de pipeline.

### Funcionalidades

- Inicialização dos registradores
- Atualização entre ciclos de clock
- Controle de stalls
- Controle de flush

---

## 📄 `pipeline.h`

Header contendo as funções públicas do pipeline.

---

## 📄 `main.c`

Arquivo principal responsável por:

- Inicializar o simulador
- Inicializar o pipeline
- Carregar instruções
- Exibir o estado inicial da arquitetura

---

# 🧠 Arquitetura Simulada

O simulador utiliza uma arquitetura baseada no pipeline clássico MIPS de 5 estágios:

```txt
IF  →  ID  →  EX  →  MEM  →  WB
```

| Estágio | Descrição |
|---|---|
| IF | Busca da instrução |
| ID | Decodificação |
| EX | Execução da ALU |
| MEM | Acesso à memória |
| WB | Escrita no banco de registradores |

---

# 💾 Configurações da Memória

| Componente | Tamanho |
|---|---|
| Memória de instruções | 1024 bytes |
| Memória de dados | 1024 bytes |
| Registradores | 32 |
| Tamanho da instrução | 4 bytes |

---

# ▶️ Como Compilar e Executar

## ✅ Pré-requisitos

Certifique-se de possuir:

- GCC instalado
- VS Code (opcional)
- Extensão C/C++ da Microsoft

Para verificar se o GCC está instalado:

```bash
gcc --version
```

---

# 🪟 Windows (PowerShell)

## Compilar

```bash
gcc main.c memory.c pipeline.c -o simulador
```

## Executar

```bash
.\simulador.exe
```

---

# 🐧 Linux

## Compilar

```bash
gcc main.c memory.c pipeline.c -o simulador
```

## Executar

```bash
./simulador
```

---

# 📌 Instruções carregadas inicialmente

```assembly
ADDI $t0, $zero, 10
ADD  $s0, $t0, $zero
SW   $s0, 0($t1)
```

Além de instruções `NOP` para esvaziamento inicial do pipeline.

---

# 📤 Saída Esperada

```txt
===== ESTADO INICIAL =====

PC = 0

IF/ID NOP  = 1
ID/EX NOP  = 1
EX/MEM NOP = 1
MEM/WB NOP = 1

Memoria de Instrucoes:
INST[0] = 0x2008000A
INST[1] = 0x01008020
INST[2] = 0xAD300000
```

Essa saída indica que:

- O PC foi inicializado corretamente
- O programa foi carregado na memória
- Os registradores de pipeline começaram vazios (`NOP`)
- O simulador está pronto para iniciar os ciclos de execução

---

# 🛠️ Tecnologias Utilizadas

- Linguagem C
- GCC
- VS Code
- Git/GitHub

---

# 🎯 Objetivos Acadêmicos

Este projeto foi desenvolvido com foco em:

- Organização modular em C
- Simulação de hardware
- Arquitetura de computadores
- Pipeline de processadores
- Gerenciamento de memória
- Controle de execução
- Modelagem de registradores

---

# 👩‍💻 Autores

- Amanda Caroline Oliveira Jorge
- Caua
- Salomao
