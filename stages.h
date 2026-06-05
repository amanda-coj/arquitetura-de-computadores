// stages.h
#ifndef STAGES_H
#define STAGES_H
#include "types.h"

void stage_IF(void);
void stage_ID(void);
void stage_EX(void);
void stage_MEM(void);
void stage_WB(void);

void stages_print_stats(int total_cycles, int total_stalls);
void stages_reset(void);

#endif
