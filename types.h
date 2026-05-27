// types.h
#ifndef TYPES_H
#define TYPES_H

#define MEMORY_SIZE_BYTES     1024
#define NUM_REGISTERS         32
#define INSTRUCTION_SIZE_BYTES 4

extern unsigned int instruction_memory[MEMORY_SIZE_BYTES / INSTRUCTION_SIZE_BYTES];
extern unsigned int data_memory[MEMORY_SIZE_BYTES / INSTRUCTION_SIZE_BYTES];
extern unsigned int registers[NUM_REGISTERS];
extern unsigned int PC;

typedef struct {
    unsigned int instruction;
    unsigned int pc_plus_4;
    int NOP;
} IF_ID_Register;

typedef struct {
    unsigned int instruction;
    unsigned int pc_plus_4;
    unsigned int read_data1;
    unsigned int read_data2;
    unsigned int sign_extended_immediate;
    unsigned int rt_rd;
    unsigned int rs, rt, rd;
    unsigned int RegWrite, MemRead, MemWrite, MemtoReg;
    unsigned int ALUSrc, RegDst, Branch, Jump;
    int NOP;
} ID_EX_Register;

typedef struct {
    unsigned int instruction;
    unsigned int branch_target_address;
    unsigned int alu_result;
    unsigned int read_data2;
    unsigned int write_register_address;
    unsigned int RegWrite, MemRead, MemWrite, MemtoReg, Branch;
    int NOP;
} EX_MEM_Register;

typedef struct {
    unsigned int instruction;
    unsigned int read_data_from_memory;
    unsigned int alu_result;
    unsigned int write_register_address;
    unsigned int RegWrite, MemtoReg;
    int NOP;
} MEM_WB_Register;

extern IF_ID_Register  IF_ID_old,  IF_ID_new;
extern ID_EX_Register  ID_EX_old,  ID_EX_new;
extern EX_MEM_Register EX_MEM_old, EX_MEM_new;
extern MEM_WB_Register MEM_WB_old, MEM_WB_new;

extern int stall_pipeline;
extern int flush_pipeline;
extern unsigned int branch_address;

#endif