// hazard.c
#include "hazard.h"

int hazard_detect_load_use(void) {
    if (!ID_EX_old.MemRead || ID_EX_old.rt == 0)
        return 0;
    unsigned int rs = (IF_ID_old.instruction >> 21) & 0x1F;
    unsigned int rt = (IF_ID_old.instruction >> 16) & 0x1F;
    return (ID_EX_old.rt == rs || ID_EX_old.rt == rt);
}

unsigned int hazard_forward_operand(unsigned int reg_index, unsigned int current_value) {
    // forwarding de EX/MEM
    if (EX_MEM_old.RegWrite && EX_MEM_old.write_register_address != 0 &&
        EX_MEM_old.write_register_address == reg_index)
        return EX_MEM_old.alu_result;

    // forwarding de MEM/WB
    if (MEM_WB_old.RegWrite && MEM_WB_old.write_register_address != 0 &&
        MEM_WB_old.write_register_address == reg_index)
        return MEM_WB_old.MemtoReg
               ? MEM_WB_old.read_data_from_memory
               : MEM_WB_old.alu_result;

    return current_value;
}
