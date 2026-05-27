/**
 * PROJETO: Simulador de Pipeline MIPS-Lite
 * FASE 1: Definição de Estruturas e Organização do Pipeline
 * 
 * Descrição: Esta versão inicial foca na definição das estruturas de dados
 * necessárias para representar a arquitetura MIPS e os registradores de pipeline.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Configurações da Arquitetura ---
#define MEM_SIZE 1024        // 1KB de Memória
#define NUM_REGS 32          // 32 Registradores de propósito geral

// --- Memórias e Registradores ---
unsigned int instruction_memory[MEM_SIZE / 4];
unsigned int data_memory[MEM_SIZE / 4];
unsigned int registers[NUM_REGS];
unsigned int PC = 0;

// --- Registradores de Pipeline (Buffers) ---
// Cada struct representa os dados que viajam entre um estágio e outro

typedef struct {
    unsigned int instruction;
    unsigned int pc_plus_4;
} IF_ID_Buffer;

typedef struct {
    unsigned int instruction;
    unsigned int read_data1;
    unsigned int read_data2;
    unsigned int imm;
    unsigned int rs, rt, rd;
    // Sinais de Controle (Exemplo)
    int RegWrite, MemRead, MemWrite;
} ID_EX_Buffer;

typedef struct {
    unsigned int alu_result;
    unsigned int read_data2;
    unsigned int write_reg;
    int RegWrite, MemRead, MemWrite;
} EX_MEM_Buffer;

typedef struct {
    unsigned int mem_data;
    unsigned int alu_result;
    unsigned int write_reg;
    int RegWrite;
} MEM_WB_Buffer;

// Instâncias dos buffers (Estado atual e Próximo estado)
// Usamos dois de cada para simular a atualização síncrona do clock
IF_ID_Buffer IF_ID_old, IF_ID_new;
ID_EX_Buffer ID_EX_old, ID_EX_new;
EX_MEM_Buffer EX_MEM_old, EX_MEM_new;
MEM_WB_Buffer MEM_WB_old, MEM_WB_new;

// --- Protótipos das Funções dos Estágios ---
void fetch();
void decode();
void execute();
void memory();
void writeback();

void update_clock() {
    // No final de cada ciclo, os valores "novos" tornam-se "antigos"
    IF_ID_old = IF_ID_new;
    ID_EX_old = ID_EX_new;
    EX_MEM_old = EX_MEM_new;
    MEM_WB_old = MEM_WB_new;
}

int main() {
    printf("--- Simulador MIPS Fase 1: Inicializando Estruturas ---\n");
    
    // Inicialização básica
    memset(registers, 0, sizeof(registers));
    memset(instruction_memory, 0, sizeof(instruction_memory));
    
    // Carregar um NOP inicial para exemplo
    instruction_memory[0] = 0x00000000; 

    int ciclos = 0;
    while (ciclos < 5) { // Simular apenas 5 ciclos para demonstração
        ciclos++;
        printf("\nCiclo de Clock: %d\n", ciclos);

        // Os estágios são chamados. Na Fase 1, eles apenas imprimem o que fariam.
        writeback();
        memory();
        execute();
        decode();
        fetch();

        update_clock();
    }

    printf("\nFase 1 concluída com sucesso.\n");
    return 0;
}

// --- Implementações Esqueléticas (Para preencher na Fase 2) ---

void fetch() {
    printf("[IF] Buscando instrução no PC: 0x%08X\n", PC);
    IF_ID_new.instruction = instruction_memory[PC / 4];
    IF_ID_new.pc_plus_4 = PC + 4;
    PC += 4;
}

void decode() {
    printf("[ID] Decodificando instrução: 0x%08X\n", IF_ID_old.instruction);
    // Lógica de extração de campos (Opcode, Rs, Rt, Rd...)
}

void execute() {
    printf("[EX] Executando operação na ULA...\n");
}

void memory() {
    printf("[MEM] Acessando memória de dados...\n");
}

void writeback() {
    printf("[WB] Escrevendo resultado no banco de registradores...\n");
}