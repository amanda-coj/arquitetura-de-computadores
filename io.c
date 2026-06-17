// io.c
#include <stdio.h>
#include <string.h>
#include "io.h"

#define MAX_CYCLES 128

static CycleSnapshot snapshots[MAX_CYCLES];
static int           n_snapshots = 0;

// ---------------------------------------------------------------------------
// Decodificador de mnemônico
// ---------------------------------------------------------------------------

static const char *rname(unsigned int r) {
    static const char *n[32] = {
        "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
        "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
        "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
        "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"
    };
    return r < 32 ? n[r] : "?";
}

static void instr_mnemonic(unsigned int inst, char *buf, int len) {
    if (inst == 0x00000000) { snprintf(buf, len, "NOP"); return; }
    unsigned int op  = (inst >> 26) & 0x3F;
    unsigned int rs  = (inst >> 21) & 0x1F;
    unsigned int rt  = (inst >> 16) & 0x1F;
    unsigned int rd  = (inst >> 11) & 0x1F;
    int          imm = (int)(short)(inst & 0xFFFF);
    switch (op) {
        case 0x00:
            switch (inst & 0x3F) {
                case 0x20: snprintf(buf,len,"ADD %s,%s,%s",  rname(rd),rname(rs),rname(rt)); return;
                case 0x22: snprintf(buf,len,"SUB %s,%s,%s",  rname(rd),rname(rs),rname(rt)); return;
                case 0x24: snprintf(buf,len,"AND %s,%s,%s",  rname(rd),rname(rs),rname(rt)); return;
                case 0x25: snprintf(buf,len,"OR  %s,%s,%s",  rname(rd),rname(rs),rname(rt)); return;
                case 0x2A: snprintf(buf,len,"SLT %s,%s,%s",  rname(rd),rname(rs),rname(rt)); return;
                default:   snprintf(buf,len,"R?%02X", inst & 0x3F); return;
            }
        case 0x08: snprintf(buf,len,"ADDI %s,%s,%d",  rname(rt),rname(rs),imm); return;
        case 0x23: snprintf(buf,len,"LW   %s,%d(%s)", rname(rt),imm,rname(rs)); return;
        case 0x2B: snprintf(buf,len,"SW   %s,%d(%s)", rname(rt),imm,rname(rs)); return;
        case 0x04: snprintf(buf,len,"BEQ  %s,%s,%d",  rname(rs),rname(rt),imm); return;
        default:   snprintf(buf,len,"0x%08X", inst); return;
    }
}

// ---------------------------------------------------------------------------

void io_reset(void) {
    n_snapshots = 0;
}

void io_record_cycle(int cycle, int stall, int flush,
                     unsigned int wb_instr, int wb_nop) {
    if (n_snapshots >= MAX_CYCLES) return;

    CycleSnapshot *s = &snapshots[n_snapshots++];
    s->cycle  = cycle;
    s->pc     = PC;
    s->stall  = stall;
    s->flush  = flush;
    s->if_id  = IF_ID_old;
    s->id_ex  = ID_EX_old;
    s->ex_mem = EX_MEM_old;
    s->mem_wb = MEM_WB_old;
    memcpy(s->registers, registers, sizeof(registers));

    // IF: conteúdo de IF_ID após update = instrução que acabou de sair de IF
    instr_mnemonic(IF_ID_old.instruction, s->pipeline_view[0].mnemonic, 48);
    s->pipeline_view[0].active = !IF_ID_old.NOP;

    // ID: conteúdo de ID_EX após update = instrução que acabou de sair de ID
    instr_mnemonic(ID_EX_old.instruction, s->pipeline_view[1].mnemonic, 48);
    s->pipeline_view[1].active = !ID_EX_old.NOP;

    // EX: conteúdo de EX_MEM após update
    instr_mnemonic(EX_MEM_old.instruction, s->pipeline_view[2].mnemonic, 48);
    s->pipeline_view[2].active = !EX_MEM_old.NOP;

    // MEM: conteúdo de MEM_WB após update
    instr_mnemonic(MEM_WB_old.instruction, s->pipeline_view[3].mnemonic, 48);
    s->pipeline_view[3].active = !MEM_WB_old.NOP;

    // WB: capturado antes do update (instrução que acabou de escrever back)
    instr_mnemonic(wb_instr, s->pipeline_view[4].mnemonic, 48);
    s->pipeline_view[4].active = !wb_nop;
}

// ---------------------------------------------------------------------------
// Helpers de saída JSON
// ---------------------------------------------------------------------------

static void write_pipeline_view(FILE *f, CycleSnapshot *s) {
    static const char *stages[5] = {"IF","ID","EX","MEM","WB"};
    fprintf(f, "  \"pipeline_view\": [\n");
    for (int i = 0; i < 5; i++) {
        fprintf(f, "    { \"stage\": \"%s\", \"instr\": \"%s\", \"active\": %s }%s\n",
                stages[i],
                s->pipeline_view[i].active ? s->pipeline_view[i].mnemonic : "---",
                s->pipeline_view[i].active ? "true" : "false",
                i < 4 ? "," : "");
    }
    fprintf(f, "  ]");
}

static void write_if_id(FILE *f, IF_ID_Register *r) {
    fprintf(f, "  \"if_id\": {\n");
    fprintf(f, "    \"instruction\": \"0x%08X\",\n", r->instruction);
    fprintf(f, "    \"pc_plus_4\": %u,\n",            r->pc_plus_4);
    fprintf(f, "    \"nop\": %d\n",                   r->NOP);
    fprintf(f, "  }");
}

static void write_id_ex(FILE *f, ID_EX_Register *r) {
    fprintf(f, "  \"id_ex\": {\n");
    fprintf(f, "    \"instruction\": \"0x%08X\",\n",  r->instruction);
    fprintf(f, "    \"read_data1\": %u,\n",            r->read_data1);
    fprintf(f, "    \"read_data2\": %u,\n",            r->read_data2);
    fprintf(f, "    \"imm\": %d,\n",                   (int)r->sign_extended_immediate);
    fprintf(f, "    \"rs\": %u,\n",                    r->rs);
    fprintf(f, "    \"rt\": %u,\n",                    r->rt);
    fprintf(f, "    \"rd\": %u,\n",                    r->rd);
    fprintf(f, "    \"rt_rd\": %u,\n",                 r->rt_rd);
    fprintf(f, "    \"RegWrite\": %u,\n",              r->RegWrite);
    fprintf(f, "    \"ALUSrc\": %u,\n",                r->ALUSrc);
    fprintf(f, "    \"MemRead\": %u,\n",               r->MemRead);
    fprintf(f, "    \"MemWrite\": %u,\n",              r->MemWrite);
    fprintf(f, "    \"MemtoReg\": %u,\n",              r->MemtoReg);
    fprintf(f, "    \"Branch\": %u,\n",                r->Branch);
    fprintf(f, "    \"nop\": %d\n",                    r->NOP);
    fprintf(f, "  }");
}

static void write_ex_mem(FILE *f, EX_MEM_Register *r) {
    fprintf(f, "  \"ex_mem\": {\n");
    fprintf(f, "    \"instruction\": \"0x%08X\",\n",  r->instruction);
    fprintf(f, "    \"alu_result\": %u,\n",            r->alu_result);
    fprintf(f, "    \"read_data2\": %u,\n",            r->read_data2);
    fprintf(f, "    \"write_reg\": %u,\n",             r->write_register_address);
    fprintf(f, "    \"branch_target\": \"0x%08X\",\n", r->branch_target_address);
    fprintf(f, "    \"RegWrite\": %u,\n",              r->RegWrite);
    fprintf(f, "    \"MemRead\": %u,\n",               r->MemRead);
    fprintf(f, "    \"MemWrite\": %u,\n",              r->MemWrite);
    fprintf(f, "    \"MemtoReg\": %u,\n",              r->MemtoReg);
    fprintf(f, "    \"Branch\": %u,\n",                r->Branch);
    fprintf(f, "    \"nop\": %d\n",                    r->NOP);
    fprintf(f, "  }");
}

static void write_mem_wb(FILE *f, MEM_WB_Register *r) {
    fprintf(f, "  \"mem_wb\": {\n");
    fprintf(f, "    \"instruction\": \"0x%08X\",\n",  r->instruction);
    fprintf(f, "    \"alu_result\": %u,\n",            r->alu_result);
    fprintf(f, "    \"mem_data\": %u,\n",              r->read_data_from_memory);
    fprintf(f, "    \"write_reg\": %u,\n",             r->write_register_address);
    fprintf(f, "    \"RegWrite\": %u,\n",              r->RegWrite);
    fprintf(f, "    \"MemtoReg\": %u,\n",              r->MemtoReg);
    fprintf(f, "    \"nop\": %d\n",                    r->NOP);
    fprintf(f, "  }");
}

static void write_registers(FILE *f, unsigned int *regs) {
    fprintf(f, "  \"registers\": [");
    for (int i = 0; i < NUM_REGISTERS; i++) {
        fprintf(f, "%u", regs[i]);
        if (i < NUM_REGISTERS - 1) fprintf(f, ", ");
    }
    fprintf(f, "]");
}

static void write_snapshot(FILE *f, CycleSnapshot *s) {
    fprintf(f, "{\n");
    fprintf(f, "  \"cycle\": %d,\n",  s->cycle);
    fprintf(f, "  \"pc\": %u,\n",     s->pc);
    fprintf(f, "  \"stall\": %d,\n",  s->stall);
    fprintf(f, "  \"flush\": %d,\n",  s->flush);
    write_pipeline_view(f, s); fprintf(f, ",\n");
    write_if_id(f,  &s->if_id);  fprintf(f, ",\n");
    write_id_ex(f,  &s->id_ex);  fprintf(f, ",\n");
    write_ex_mem(f, &s->ex_mem); fprintf(f, ",\n");
    write_mem_wb(f, &s->mem_wb); fprintf(f, ",\n");
    write_registers(f, s->registers);
    fprintf(f, "\n}\n");
}

void io_print_last_json(void) {
    if (n_snapshots == 0) return;
    write_snapshot(stdout, &snapshots[n_snapshots - 1]);
}

void io_write_json(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "{\n  \"cycles\": [\n");
    for (int i = 0; i < n_snapshots; i++) {
        CycleSnapshot *s = &snapshots[i];
        fprintf(f, "    {\n");
        fprintf(f, "      \"cycle\": %d,\n",  s->cycle);
        fprintf(f, "      \"pc\": %u,\n",     s->pc);
        fprintf(f, "      \"stall\": %d,\n",  s->stall);
        fprintf(f, "      \"flush\": %d,\n",  s->flush);

        // pipeline_view
        fprintf(f, "      \"pipeline_view\": [\n");
        static const char *stages[5] = {"IF","ID","EX","MEM","WB"};
        for (int k = 0; k < 5; k++) {
            fprintf(f, "        { \"stage\": \"%s\", \"instr\": \"%s\", \"active\": %s }%s\n",
                    stages[k],
                    s->pipeline_view[k].active ? s->pipeline_view[k].mnemonic : "---",
                    s->pipeline_view[k].active ? "true" : "false",
                    k < 4 ? "," : "");
        }
        fprintf(f, "      ],\n");

        fprintf(f, "      \"if_id\": {\n");
        fprintf(f, "        \"instruction\": \"0x%08X\",\n", s->if_id.instruction);
        fprintf(f, "        \"pc_plus_4\": %u,\n",            s->if_id.pc_plus_4);
        fprintf(f, "        \"nop\": %d\n",                   s->if_id.NOP);
        fprintf(f, "      },\n");

        fprintf(f, "      \"id_ex\": {\n");
        fprintf(f, "        \"instruction\": \"0x%08X\",\n",  s->id_ex.instruction);
        fprintf(f, "        \"read_data1\": %u,\n",            s->id_ex.read_data1);
        fprintf(f, "        \"read_data2\": %u,\n",            s->id_ex.read_data2);
        fprintf(f, "        \"imm\": %d,\n",                   (int)s->id_ex.sign_extended_immediate);
        fprintf(f, "        \"rs\": %u,\n",                    s->id_ex.rs);
        fprintf(f, "        \"rt\": %u,\n",                    s->id_ex.rt);
        fprintf(f, "        \"rd\": %u,\n",                    s->id_ex.rd);
        fprintf(f, "        \"rt_rd\": %u,\n",                 s->id_ex.rt_rd);
        fprintf(f, "        \"RegWrite\": %u,\n",              s->id_ex.RegWrite);
        fprintf(f, "        \"ALUSrc\": %u,\n",                s->id_ex.ALUSrc);
        fprintf(f, "        \"MemRead\": %u,\n",               s->id_ex.MemRead);
        fprintf(f, "        \"MemWrite\": %u,\n",              s->id_ex.MemWrite);
        fprintf(f, "        \"MemtoReg\": %u,\n",              s->id_ex.MemtoReg);
        fprintf(f, "        \"Branch\": %u,\n",                s->id_ex.Branch);
        fprintf(f, "        \"nop\": %d\n",                    s->id_ex.NOP);
        fprintf(f, "      },\n");

        fprintf(f, "      \"ex_mem\": {\n");
        fprintf(f, "        \"instruction\": \"0x%08X\",\n",   s->ex_mem.instruction);
        fprintf(f, "        \"alu_result\": %u,\n",             s->ex_mem.alu_result);
        fprintf(f, "        \"read_data2\": %u,\n",             s->ex_mem.read_data2);
        fprintf(f, "        \"write_reg\": %u,\n",              s->ex_mem.write_register_address);
        fprintf(f, "        \"branch_target\": \"0x%08X\",\n",  s->ex_mem.branch_target_address);
        fprintf(f, "        \"RegWrite\": %u,\n",               s->ex_mem.RegWrite);
        fprintf(f, "        \"MemRead\": %u,\n",                s->ex_mem.MemRead);
        fprintf(f, "        \"MemWrite\": %u,\n",               s->ex_mem.MemWrite);
        fprintf(f, "        \"MemtoReg\": %u,\n",               s->ex_mem.MemtoReg);
        fprintf(f, "        \"Branch\": %u,\n",                 s->ex_mem.Branch);
        fprintf(f, "        \"nop\": %d\n",                     s->ex_mem.NOP);
        fprintf(f, "      },\n");

        fprintf(f, "      \"mem_wb\": {\n");
        fprintf(f, "        \"instruction\": \"0x%08X\",\n",  s->mem_wb.instruction);
        fprintf(f, "        \"alu_result\": %u,\n",            s->mem_wb.alu_result);
        fprintf(f, "        \"mem_data\": %u,\n",              s->mem_wb.read_data_from_memory);
        fprintf(f, "        \"write_reg\": %u,\n",             s->mem_wb.write_register_address);
        fprintf(f, "        \"RegWrite\": %u,\n",              s->mem_wb.RegWrite);
        fprintf(f, "        \"MemtoReg\": %u,\n",              s->mem_wb.MemtoReg);
        fprintf(f, "        \"nop\": %d\n",                    s->mem_wb.NOP);
        fprintf(f, "      },\n");

        fprintf(f, "      \"registers\": [");
        for (int j = 0; j < NUM_REGISTERS; j++) {
            fprintf(f, "%u", s->registers[j]);
            if (j < NUM_REGISTERS - 1) fprintf(f, ", ");
        }
        fprintf(f, "]\n");

        fprintf(f, "    }");
        if (i < n_snapshots - 1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, "  ]\n}\n");
    fclose(f);
}
