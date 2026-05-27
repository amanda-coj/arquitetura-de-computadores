// pipeline.c
#include <string.h>
#include "pipeline.h"

IF_ID_Register  IF_ID_old,  IF_ID_new;
ID_EX_Register  ID_EX_old,  ID_EX_new;
EX_MEM_Register EX_MEM_old, EX_MEM_new;
MEM_WB_Register MEM_WB_old, MEM_WB_new;

int stall_pipeline  = 0;
int flush_pipeline  = 0;
unsigned int branch_address = 0;

void pipeline_registers_init(void) {
    memset(&IF_ID_old,  0, sizeof(IF_ID_old));
    memset(&IF_ID_new,  0, sizeof(IF_ID_new));
    memset(&ID_EX_old,  0, sizeof(ID_EX_old));
    memset(&ID_EX_new,  0, sizeof(ID_EX_new));
    memset(&EX_MEM_old, 0, sizeof(EX_MEM_old));
    memset(&EX_MEM_new, 0, sizeof(EX_MEM_new));
    memset(&MEM_WB_old, 0, sizeof(MEM_WB_old));
    memset(&MEM_WB_new, 0, sizeof(MEM_WB_new));

    IF_ID_old.NOP  = 1;
    ID_EX_old.NOP  = 1;
    EX_MEM_old.NOP = 1;
    MEM_WB_old.NOP = 1;
}

void pipeline_registers_update(void) {
    IF_ID_old  = IF_ID_new;
    ID_EX_old  = ID_EX_new;
    EX_MEM_old = EX_MEM_new;
    MEM_WB_old = MEM_WB_new;
}