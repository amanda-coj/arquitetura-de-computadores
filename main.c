// main.c
#include <stdio.h>
#include "memory.h"
#include "pipeline.h"
#include "stages.h"
#include "hazard.h"
#include "types.h"

static int all_nop(void) {
    return IF_ID_old.NOP && ID_EX_old.NOP && EX_MEM_old.NOP && MEM_WB_old.NOP;
}

int main(void) {
    memory_init();
    pipeline_registers_init();
    load_program();

    int cycle        = 0;
    int total_stalls = 0;

    while (!(PC >= 28 && all_nop())) {
        stall_pipeline = hazard_detect_load_use();
        if (stall_pipeline) total_stalls++;

        stage_WB();
        stage_MEM();
        stage_EX();
        stage_ID();
        stage_IF();

        pipeline_registers_update();
        cycle++;
    }

    stages_print_stats(cycle, total_stalls);

    printf("===== ESTADO FINAL DOS REGISTRADORES =====\n\n");
    printf("  $t0 ($8)  = %u\n",    registers[8]);
    printf("  $t1 ($9)  = 0x%X\n",  registers[9]);
    printf("  $s0 ($16) = %u\n\n",  registers[16]);

    printf("  mem[0x%X] = %u\n", registers[9], data_memory[registers[9] / 4]);

    return 0;
}
