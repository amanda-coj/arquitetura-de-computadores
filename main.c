// main.c
#include <stdio.h>
#include "memory.h"
#include "types.h"

int main() {

    // Inicializa memórias e registradores
    memory_init();

    // Carrega instruções na memória
    load_program();

    // Mostra estado inicial
    printf("===== ESTADO INICIAL =====\n\n");

    printf("PC = %u\n\n", PC);

    printf("Registradores:\n");
    for(int i = 0; i < NUM_REGISTERS; i++) {
        printf("R[%d] = %u\n", i, registers[i]);
    }

    printf("\nMemoria de Instrucoes:\n");
    for(int i = 0; i < 7; i++) {
        printf("INST[%d] = 0x%08X\n", i, instruction_memory[i]);
    }

    return 0;
}