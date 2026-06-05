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
static int sim_done     = 0;

static int all_nop(void) {
    return IF_ID_old.NOP && ID_EX_old.NOP && EX_MEM_old.NOP && MEM_WB_old.NOP;
}

static void do_reset(void) {
    memory_init();
    pipeline_registers_init();
    load_program();
    stall_pipeline = 0;
    flush_pipeline = 0;
    branch_address = 0;
    cycle          = 0;
    total_stalls   = 0;
    sim_done       = 0;
    io_reset();
    stages_reset();
}

static void run_one_cycle(void) {
    stall_pipeline = hazard_detect_load_use();
    if (stall_pipeline) total_stalls++;

    stage_WB(); stage_MEM(); stage_EX(); stage_ID(); stage_IF();

    pipeline_registers_update();
    io_record_cycle(cycle, stall_pipeline, flush_pipeline);
    cycle++;

    if (PC >= PROGRAM_END && all_nop()) sim_done = 1;
}

static void print_final_state(void) {
    printf("===== ESTADO FINAL DOS REGISTRADORES =====\n\n");
    printf("  $t0 ($8)  = %u\n",   registers[8]);
    printf("  $t1 ($9)  = 0x%X\n", registers[9]);
    printf("  $s0 ($16) = %u\n",   registers[16]);
    printf("  $t2 ($10) = %u\n",   registers[10]);
    printf("  $t3 ($11) = %u\n",   registers[11]);
    printf("  $s1 ($17) = %u  (nao escrito, ADD descartado pelo flush)\n\n", registers[17]);
    printf("  mem[0x%X] = %u\n", registers[9], data_memory[registers[9] / 4]);
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
            }
            printf("END\n");
            fflush(stdout);

        } else if (strcmp(cmd, "run_all") == 0) {
            while (!sim_done) run_one_cycle();
            io_write_json("pipeline_trace.json");
            stages_print_stats(cycle, total_stalls);
            print_final_state();
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
