// io.h
#ifndef IO_H
#define IO_H
#include "types.h"

typedef struct {
    char mnemonic[48]; // ex: "ADDI $t0,$zero,10"
    int  active;       // 1 = instrucao real, 0 = NOP/bolha
} PipelineSlot;

typedef struct {
    int          cycle;
    unsigned int pc;
    int          stall, flush;
    IF_ID_Register  if_id;
    ID_EX_Register  id_ex;
    EX_MEM_Register ex_mem;
    MEM_WB_Register mem_wb;
    unsigned int    registers[NUM_REGISTERS];
    PipelineSlot    pipeline_view[5]; // [0]=IF [1]=ID [2]=EX [3]=MEM [4]=WB
} CycleSnapshot;

void io_record_cycle(int cycle, int stall, int flush,
                     unsigned int wb_instr, int wb_nop);
void io_write_json(const char *path);
void io_print_last_json(void);
void io_reset(void);

#endif
