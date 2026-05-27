// main.c
#include <stdio.h>
#include "memory.h"
#include "pipeline.h"
#include "types.h"

int main() {

    memory_init();

    pipeline_registers_init();

    load_program();

    printf("===== ESTADO INICIAL =====\n\n");

    printf("PC = %u\n\n", PC);

    printf("IF/ID NOP  = %d\n", IF_ID_old.NOP);
    printf("ID/EX NOP  = %d\n", ID_EX_old.NOP);
    printf("EX/MEM NOP = %d\n", EX_MEM_old.NOP);
    printf("MEM/WB NOP = %d\n\n", MEM_WB_old.NOP);

    printf("Memoria de Instrucoes:\n");

    for(int i = 0; i < 7; i++) {
        printf("INST[%d] = 0x%08X\n", i, instruction_memory[i]);
    }

    return 0;
}