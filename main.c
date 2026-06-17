// main.c
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "pipeline.h"
#include "stages.h"
#include "hazard.h"
#include "io.h"
#include "types.h"

#define PROGRAM_END 44   // 11 instrucoes * 4 bytes

static int cycle        = 0;
static int total_stalls = 0;
static int total_flushes = 0;
static int sim_done     = 0;

static unsigned int initial_registers[NUM_REGISTERS];

static const char *reg_names[32] = {
    "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
    "$t0",  "$t1","$t2","$t3","$t4","$t5","$t6","$t7",
    "$s0",  "$s1","$s2","$s3","$s4","$s5","$s6","$s7",
    "$t8",  "$t9","$k0","$k1","$gp","$sp","$fp","$ra"
};

static int all_nop(void) {
    return IF_ID_old.NOP && ID_EX_old.NOP && EX_MEM_old.NOP && MEM_WB_old.NOP;
}

static void do_reset(void) {
    memory_init();
    pipeline_registers_init();
    load_program();
    memcpy(initial_registers, registers, sizeof(initial_registers));
    stall_pipeline = 0;
    flush_pipeline = 0;
    branch_address = 0;
    cycle          = 0;
    total_stalls   = 0;
    total_flushes  = 0;
    sim_done       = 0;
    io_reset();
    stages_reset();
}

static void run_one_cycle(void) {
    stall_pipeline = hazard_detect_load_use();
    if (stall_pipeline) total_stalls++;

    stage_WB(); stage_MEM(); stage_EX(); stage_ID();

    int did_flush = flush_pipeline;   // stage_IF vai zerar flush_pipeline
    if (did_flush) total_flushes++;
    stage_IF();

    unsigned int wb_instr = MEM_WB_old.instruction;  // captura antes do update
    int          wb_nop   = MEM_WB_old.NOP;

    pipeline_registers_update();
    io_record_cycle(cycle, stall_pipeline, did_flush, wb_instr, wb_nop);
    cycle++;

    if (PC >= PROGRAM_END && all_nop()) sim_done = 1;
}

static void print_final_summary(void) {
    printf("\n==========================================\n");
    printf("   O QUE ACONTECEU NO PIPELINE\n");
    printf("==========================================\n\n");

    printf("O processador executou %d ciclos de clock", cycle);
    if (total_stalls > 0 || total_flushes > 0) {
        printf(", incluindo");
        if (total_stalls > 0)
            printf(" %d stall%s por dependencia de dados (load-use)",
                   total_stalls, total_stalls > 1 ? "s" : "");
        if (total_stalls > 0 && total_flushes > 0) printf(" e");
        if (total_flushes > 0)
            printf(" %d flush%s por desvio condicional (BEQ tomado)",
                   total_flushes, total_flushes > 1 ? "es" : "");
    }
    printf(".\n\n");

    // Registradores que mudaram
    int changed = 0;
    for (int i = 1; i < NUM_REGISTERS; i++) {
        if (registers[i] != initial_registers[i]) changed++;
    }

    if (changed > 0) {
        printf("Registradores modificados pelo pipeline:\n\n");
        for (int i = 1; i < NUM_REGISTERS; i++) {
            if (registers[i] != initial_registers[i]) {
                printf("  %-6s ($%2d): %u  ->  %u\n",
                       reg_names[i], i, initial_registers[i], registers[i]);
            }
        }
    } else {
        printf("Nenhum registrador foi modificado.\n");
    }

    // Registradores que foram definidos no inicio mas nao mudaram
    int kept = 0;
    for (int i = 1; i < NUM_REGISTERS; i++) {
        if (registers[i] == initial_registers[i] && initial_registers[i] != 0) kept++;
    }
    if (kept > 0) {
        printf("\nRegistradores com valor inicial mantido (nao escritos pelo programa):\n\n");
        for (int i = 1; i < NUM_REGISTERS; i++) {
            if (registers[i] == initial_registers[i] && initial_registers[i] != 0) {
                printf("  %-6s ($%2d): %u  (sem alteracao)\n",
                       reg_names[i], i, registers[i]);
            }
        }
    }

    // Memoria de dados alterada
    printf("\nMemoria de dados escrita pelo programa:\n\n");
    int mem_written = 0;
    for (int i = 0; i < MEMORY_SIZE_BYTES / INSTRUCTION_SIZE_BYTES; i++) {
        if (data_memory[i] != 0xFFFFFFFF) {
            printf("  mem[0x%03X] = %u\n", i * 4, data_memory[i]);
            mem_written++;
        }
    }
    if (!mem_written)
        printf("  (nenhuma posicao foi escrita)\n");

    printf("\n");
}

int main(void) {
    do_reset();

    fprintf(stderr, "Simulador MIPS Pipeline\n");
    fprintf(stderr, "Comandos: next | run_all | reset | exit\n");

    char cmd[64];
    while (fgets(cmd, sizeof(cmd), stdin)) {
        int len = (int)strlen(cmd);
        if (len > 0 && cmd[len - 1] == '\n') cmd[--len] = '\0';
        if (len > 0 && cmd[len - 1] == '\r') cmd[--len] = '\0';

        if (strcmp(cmd, "next") == 0) {
            if (sim_done) {
                printf("{\"status\":\"done\"}\n");
            } else {
                run_one_cycle();
                io_print_last_json();
                if (sim_done) print_final_summary();
            }
            printf("END\n");
            fflush(stdout);

        } else if (strcmp(cmd, "run_all") == 0) {
            while (!sim_done) run_one_cycle();
            io_write_json("pipeline_trace.json");
            stages_print_stats(cycle, total_stalls);
            print_final_summary();
            printf("END\n");
            fflush(stdout);

        } else if (strcmp(cmd, "reset") == 0) {
            do_reset();
            printf("{\"status\":\"reset\"}\n");
            printf("END\n");
            fflush(stdout);

        } else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            break;
        }

        fprintf(stderr, "> ");
    }

    return 0;
}
