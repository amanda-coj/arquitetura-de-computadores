// io.c
#include <stdio.h>
#include <string.h>
#include "io.h"

#define MAX_CYCLES 128

static CycleSnapshot snapshots[MAX_CYCLES];
static int           n_snapshots = 0;

void io_reset(void) {
    n_snapshots = 0;
}

void io_record_cycle(int cycle, int stall, int flush) {
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
}

// ---------------------------------------------------------------------------

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
        // reindenta para encaixar dentro do array
        CycleSnapshot *s = &snapshots[i];
        fprintf(f, "    {\n");
        fprintf(f, "      \"cycle\": %d,\n",  s->cycle);
        fprintf(f, "      \"pc\": %u,\n",     s->pc);
        fprintf(f, "      \"stall\": %d,\n",  s->stall);
        fprintf(f, "      \"flush\": %d,\n",  s->flush);

        // reutiliza helpers com indentacao extra via fprintf direto
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
