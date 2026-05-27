// memory.c
#include <string.h>
#include "memory.h"

unsigned int instruction_memory[MEMORY_SIZE_BYTES / INSTRUCTION_SIZE_BYTES];
unsigned int data_memory       [MEMORY_SIZE_BYTES / INSTRUCTION_SIZE_BYTES];
unsigned int registers[NUM_REGISTERS];
unsigned int PC = 0;

void memory_init(void) {
    memset(instruction_memory, 0, sizeof(instruction_memory));
    for (int i = 0; i < MEMORY_SIZE_BYTES / INSTRUCTION_SIZE_BYTES; i++)
        data_memory[i] = 0xFFFFFFFF;
    memset(registers, 0, sizeof(registers));
    PC = 0;
}

void load_program(void) {
    instruction_memory[0] = 0x2008000A; // ADDI $t0, $zero, 10
    instruction_memory[1] = 0x01008020; // ADD  $s0, $t0, $zero
    instruction_memory[2] = 0xAD300000; // SW   $s0, 0($t1)
    // NOPs para esvaziar o pipeline
    instruction_memory[3] = 0x00000000;
    instruction_memory[4] = 0x00000000;
    instruction_memory[5] = 0x00000000;
    instruction_memory[6] = 0x00000000;

    registers[0]  = 0;
    registers[9]  = 0x100; // $t1 aponta para endereço 256
    registers[16] = 0;
}