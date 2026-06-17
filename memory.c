// memory.c

#include <string.h>
#include "memory.h"

unsigned int instruction_memory[MEMORY_SIZE_BYTES / INSTRUCTION_SIZE_BYTES];
unsigned int data_memory[MEMORY_SIZE_BYTES / INSTRUCTION_SIZE_BYTES];
unsigned int registers[NUM_REGISTERS];
unsigned int PC = 0;

void memory_init(void) {

    memset(instruction_memory, 0, sizeof(instruction_memory));

    for (int i = 0; i < MEMORY_SIZE_BYTES / INSTRUCTION_SIZE_BYTES; i++) {
        data_memory[i] = 0xFFFFFFFF;
    }

    memset(registers, 0, sizeof(registers));

    PC = 0;
}

void load_program(void) {

    // ADDI $t0, $zero, 10
    instruction_memory[0] = 0x2008000A;

    // ADD $s0, $t0, $zero
    instruction_memory[1] = 0x01008020;

    // SW $s0, 0($zero)
    instruction_memory[2] = 0xAC100000;

    // LW $t2, 0($zero)        -> $t2 = mem[0x000] = 10  (load-use stall com inst seguinte)
    instruction_memory[3] = 0x8C0A0000;

    // ADDI $t3, $t2, 5       -> $t3 = 15  (load-use stall: usa $t2 logo apos LW)
    instruction_memory[4] = 0x214B0005;

    // BEQ $t0, $t2, 1        -> branch tomado ($t0=$t2=10), flush
    instruction_memory[5] = 0x110A0001;

    // ADD $s1, $t3, $zero    -> nunca executa (alvo do flush)
    instruction_memory[6] = 0x01608820;

    // NOPs para esvaziar pipeline
    instruction_memory[7]  = 0x00000000;
    instruction_memory[8]  = 0x00000000;
    instruction_memory[9]  = 0x00000000;
    instruction_memory[10] = 0x00000000;

    registers[0] = 0; // $zero (sempre 0)
}