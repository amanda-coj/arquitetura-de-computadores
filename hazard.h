// hazard.h
#ifndef HAZARD_H
#define HAZARD_H
#include "types.h"

int          hazard_detect_load_use(void);
unsigned int hazard_forward_operand(unsigned int reg_index, unsigned int current_value);

#endif
