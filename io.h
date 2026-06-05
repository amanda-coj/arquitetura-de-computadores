// io.h
#ifndef IO_H
#define IO_H
#include "types.h"

typedef struct {
    int          cycle;
    unsigned int pc;
    int          stall, flush;
    IF_ID_Register  if_id;
    ID_EX_Register  id_ex;
    EX_MEM_Register ex_mem;
    MEM_WB_Register mem_wb;
    unsigned int registers[NUM_REGISTERS];
} CycleSnapshot;

void io_record_cycle(int cycle, int stall, int flush);
void io_write_json(const char *path);
void io_print_last_json(void);
void io_reset(void);

#endif
