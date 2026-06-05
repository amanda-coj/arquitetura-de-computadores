// stages.c
#include <stdio.h>
#include <string.h>
#include "stages.h"
#include "hazard.h"

// ---------------------------------------------------------------------------
// Rastreamento por instrucao
// ---------------------------------------------------------------------------

#define MAX_INSTR 16

typedef struct {
    unsigned int inst;
    // IF
    unsigned int if_pc;
    // ID
    unsigned int id_rs, id_rt, id_rd, id_dest;
    unsigned int id_data1, id_data2;
    int          id_imm;
    unsigned int id_opcode;
    unsigned int id_RegWrite, id_ALUSrc, id_MemRead, id_MemWrite, id_MemtoReg, id_Branch;
    // EX
    unsigned int ex_op1, ex_op2, ex_result, ex_dest;
    int          ex_branch_taken;
    // MEM
    int          mem_op; // 0=passthrough 1=read 2=write
    unsigned int mem_addr, mem_rdata, mem_wdata;
    // WB
    int          wb_written;
    unsigned int wb_reg, wb_value;
} InstrTrace;

static InstrTrace traces[MAX_INSTR];
static int        n_traces = 0;

// contadores globais
static int if_fetch, if_stall, if_flush;
static int ex_branch_taken_total;

// ---------------------------------------------------------------------------

static const char *reg_name(unsigned int r) {
    static const char *n[32] = {
        "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
        "$t0",  "$t1","$t2","$t3","$t4","$t5","$t6","$t7",
        "$s0",  "$s1","$s2","$s3","$s4","$s5","$s6","$s7",
        "$t8",  "$t9","$k0","$k1","$gp","$sp","$fp","$ra"
    };
    return r < 32 ? n[r] : "?";
}

static int find_trace(unsigned int inst) {
    for (int i = 0; i < n_traces; i++)
        if (traces[i].inst == inst) return i;
    return -1;
}

// ---------------------------------------------------------------------------
// Estagios
// ---------------------------------------------------------------------------

void stage_IF(void) {
    if (stall_pipeline) {
        IF_ID_new = IF_ID_old;
        if_stall++;
        return;
    }
    if (flush_pipeline) {
        // remove trace da instrucao que estava em IF/ID e morreu pelo branch
        unsigned int killed = IF_ID_old.instruction;
        if (killed != 0) {
            int idx = find_trace(killed);
            if (idx >= 0) {
                for (int i = idx; i < n_traces - 1; i++)
                    traces[i] = traces[i + 1];
                n_traces--;
            }
        }
        memset(&IF_ID_new, 0, sizeof(IF_ID_new));
        IF_ID_new.NOP  = 1;
        PC             = branch_address;
        flush_pipeline = 0;
        if_flush++;
        return;
    }

    unsigned int inst = instruction_memory[PC / 4];
    IF_ID_new.instruction = inst;
    IF_ID_new.pc_plus_4   = PC + 4;
    IF_ID_new.NOP         = (inst == 0x00000000) ? 1 : 0;

    // registra trace apenas para instrucoes reais
    if (inst != 0x00000000 && n_traces < MAX_INSTR) {
        traces[n_traces].inst  = inst;
        traces[n_traces].if_pc = PC;
        n_traces++;
    }

    if_fetch++;
    PC += 4;
}

void stage_ID(void) {
    if (IF_ID_old.NOP || stall_pipeline || flush_pipeline) {
        memset(&ID_EX_new, 0, sizeof(ID_EX_new));
        ID_EX_new.NOP = 1;
        return;
    }

    unsigned int inst   = IF_ID_old.instruction;
    unsigned int opcode = (inst >> 26) & 0x3F;
    unsigned int rs     = (inst >> 21) & 0x1F;
    unsigned int rt     = (inst >> 16) & 0x1F;
    unsigned int rd     = (inst >> 11) & 0x1F;

    ID_EX_new.instruction             = inst;
    ID_EX_new.pc_plus_4               = IF_ID_old.pc_plus_4;
    ID_EX_new.rs                      = rs;
    ID_EX_new.rt                      = rt;
    ID_EX_new.rd                      = rd;
    ID_EX_new.read_data1              = registers[rs];
    ID_EX_new.read_data2              = registers[rt];
    ID_EX_new.sign_extended_immediate = (unsigned int)(int)(short)(inst & 0xFFFF);
    ID_EX_new.NOP                     = 0;

    // sinais de controle
    switch (opcode) {
        case 0x00: // R-type
            ID_EX_new.RegDst = 1; ID_EX_new.ALUSrc   = 0;
            ID_EX_new.MemRead = 0; ID_EX_new.MemWrite = 0;
            ID_EX_new.RegWrite = 1; ID_EX_new.MemtoReg = 0;
            ID_EX_new.Branch = 0; ID_EX_new.Jump = 0;
            break;
        case 0x08: // ADDI
            ID_EX_new.RegDst = 0; ID_EX_new.ALUSrc   = 1;
            ID_EX_new.MemRead = 0; ID_EX_new.MemWrite = 0;
            ID_EX_new.RegWrite = 1; ID_EX_new.MemtoReg = 0;
            ID_EX_new.Branch = 0; ID_EX_new.Jump = 0;
            break;
        case 0x23: // LW
            ID_EX_new.RegDst = 0; ID_EX_new.ALUSrc   = 1;
            ID_EX_new.MemRead = 1; ID_EX_new.MemWrite = 0;
            ID_EX_new.RegWrite = 1; ID_EX_new.MemtoReg = 1;
            ID_EX_new.Branch = 0; ID_EX_new.Jump = 0;
            break;
        case 0x2B: // SW
            ID_EX_new.RegDst = 0; ID_EX_new.ALUSrc   = 1;
            ID_EX_new.MemRead = 0; ID_EX_new.MemWrite = 1;
            ID_EX_new.RegWrite = 0; ID_EX_new.MemtoReg = 0;
            ID_EX_new.Branch = 0; ID_EX_new.Jump = 0;
            break;
        case 0x04: // BEQ
            ID_EX_new.RegDst = 0; ID_EX_new.ALUSrc   = 0;
            ID_EX_new.MemRead = 0; ID_EX_new.MemWrite = 0;
            ID_EX_new.RegWrite = 0; ID_EX_new.MemtoReg = 0;
            ID_EX_new.Branch = 1; ID_EX_new.Jump = 0;
            break;
        default:
            ID_EX_new.NOP = 1;
            return;
    }

    // registrador destino resolvido: rd (R-type) ou rt (I-type)
    ID_EX_new.rt_rd = ID_EX_new.RegDst ? rd : rt;

    // grava trace
    int idx = find_trace(inst);
    if (idx >= 0) {
        InstrTrace *t   = &traces[idx];
        t->id_opcode    = opcode;
        t->id_rs        = rs;
        t->id_rt        = rt;
        t->id_rd        = rd;
        t->id_dest      = ID_EX_new.rt_rd;
        t->id_data1     = ID_EX_new.read_data1;
        t->id_data2     = ID_EX_new.read_data2;
        t->id_imm       = (int)(short)(inst & 0xFFFF);
        t->id_RegWrite  = ID_EX_new.RegWrite;
        t->id_ALUSrc    = ID_EX_new.ALUSrc;
        t->id_MemRead   = ID_EX_new.MemRead;
        t->id_MemWrite  = ID_EX_new.MemWrite;
        t->id_MemtoReg  = ID_EX_new.MemtoReg;
        t->id_Branch    = ID_EX_new.Branch;
    }
}

void stage_EX(void) {
    if (ID_EX_old.NOP) {
        memset(&EX_MEM_new, 0, sizeof(EX_MEM_new));
        EX_MEM_new.NOP = 1;
        return;
    }

    unsigned int op1     = hazard_forward_operand(ID_EX_old.rs, ID_EX_old.read_data1);
    unsigned int op2_raw = hazard_forward_operand(ID_EX_old.rt, ID_EX_old.read_data2);
    unsigned int op2     = ID_EX_old.ALUSrc ? ID_EX_old.sign_extended_immediate : op2_raw;

    unsigned int opcode     = (ID_EX_old.instruction >> 26) & 0x3F;
    unsigned int funct      = ID_EX_old.instruction & 0x3F;
    unsigned int alu_result = 0;

    if (opcode == 0x00) {
        // R-type: despacha por funct
        switch (funct) {
            case 0x20: alu_result = op1 + op2; break; // ADD
            case 0x22: alu_result = op1 - op2; break; // SUB
            case 0x24: alu_result = op1 & op2; break; // AND
            case 0x25: alu_result = op1 | op2; break; // OR
            case 0x2A: alu_result = ((int)op1 < (int)op2) ? 1 : 0; break; // SLT
            default:   alu_result = op1 + op2; break;
        }
    } else {
        // I-type: ADD implicito (ADDI, LW, SW, BEQ)
        alu_result = op1 + op2;
    }

    EX_MEM_new.instruction            = ID_EX_old.instruction;
    EX_MEM_new.alu_result             = alu_result;
    EX_MEM_new.read_data2             = op2_raw;
    EX_MEM_new.write_register_address = ID_EX_old.rt_rd;
    EX_MEM_new.branch_target_address  = ID_EX_old.pc_plus_4 + (ID_EX_old.sign_extended_immediate << 2);
    EX_MEM_new.RegWrite               = ID_EX_old.RegWrite;
    EX_MEM_new.MemRead                = ID_EX_old.MemRead;
    EX_MEM_new.MemWrite               = ID_EX_old.MemWrite;
    EX_MEM_new.MemtoReg               = ID_EX_old.MemtoReg;
    EX_MEM_new.Branch                 = ID_EX_old.Branch;
    EX_MEM_new.NOP                    = 0;

    // grava trace
    int idx = find_trace(ID_EX_old.instruction);
    if (idx >= 0) {
        InstrTrace *t      = &traces[idx];
        t->ex_op1          = op1;
        t->ex_op2          = op2;
        t->ex_result       = alu_result;
        t->ex_dest         = ID_EX_old.rt_rd;
        t->ex_branch_taken = 0;
    }

    // flush por branch tomado
    if (ID_EX_old.Branch && op1 == op2) {
        flush_pipeline = 1;
        branch_address = EX_MEM_new.branch_target_address;
        ex_branch_taken_total++;
        if (idx >= 0) traces[idx].ex_branch_taken = 1;
    }
}

void stage_MEM(void) {
    if (EX_MEM_old.NOP) {
        memset(&MEM_WB_new, 0, sizeof(MEM_WB_new));
        MEM_WB_new.NOP = 1;
        return;
    }

    MEM_WB_new.instruction            = EX_MEM_old.instruction;
    MEM_WB_new.alu_result             = EX_MEM_old.alu_result;
    MEM_WB_new.write_register_address = EX_MEM_old.write_register_address;
    MEM_WB_new.RegWrite               = EX_MEM_old.RegWrite;
    MEM_WB_new.MemtoReg               = EX_MEM_old.MemtoReg;
    MEM_WB_new.NOP                    = 0;

    int idx = find_trace(EX_MEM_old.instruction);

    if (EX_MEM_old.MemRead) {
        MEM_WB_new.read_data_from_memory = data_memory[EX_MEM_old.alu_result / 4];
        if (idx >= 0) {
            traces[idx].mem_op    = 1;
            traces[idx].mem_addr  = EX_MEM_old.alu_result;
            traces[idx].mem_rdata = MEM_WB_new.read_data_from_memory;
        }
    } else if (EX_MEM_old.MemWrite) {
        data_memory[EX_MEM_old.alu_result / 4] = EX_MEM_old.read_data2;
        if (idx >= 0) {
            traces[idx].mem_op    = 2;
            traces[idx].mem_addr  = EX_MEM_old.alu_result;
            traces[idx].mem_wdata = EX_MEM_old.read_data2;
        }
    } else {
        if (idx >= 0) traces[idx].mem_op = 0;
    }
}

void stage_WB(void) {
    if (MEM_WB_old.NOP || !MEM_WB_old.RegWrite) {
        return;
    }

    unsigned int value = MEM_WB_old.MemtoReg
                         ? MEM_WB_old.read_data_from_memory
                         : MEM_WB_old.alu_result;

    int idx = find_trace(MEM_WB_old.instruction);
    if (idx >= 0) {
        traces[idx].wb_written = 1;
        traces[idx].wb_reg     = MEM_WB_old.write_register_address;
        traces[idx].wb_value   = value;
    }

    if (MEM_WB_old.write_register_address != 0)
        registers[MEM_WB_old.write_register_address] = value;
}

// ---------------------------------------------------------------------------
// Impressao do relatorio final
// ---------------------------------------------------------------------------

static void print_mnemonic(InstrTrace *t) {
    unsigned int opcode = t->id_opcode;
    unsigned int funct  = t->inst & 0x3F;
    if (opcode == 0x00) {
        switch (funct) {
            case 0x20: printf("ADD  %s, %s, %s",   reg_name(t->id_rd), reg_name(t->id_rs), reg_name(t->id_rt)); return;
            case 0x22: printf("SUB  %s, %s, %s",   reg_name(t->id_rd), reg_name(t->id_rs), reg_name(t->id_rt)); return;
            case 0x24: printf("AND  %s, %s, %s",   reg_name(t->id_rd), reg_name(t->id_rs), reg_name(t->id_rt)); return;
            case 0x25: printf("OR   %s, %s, %s",   reg_name(t->id_rd), reg_name(t->id_rs), reg_name(t->id_rt)); return;
            case 0x2A: printf("SLT  %s, %s, %s",   reg_name(t->id_rd), reg_name(t->id_rs), reg_name(t->id_rt)); return;
            default:   printf("R-type funct=0x%02X", funct); return;
        }
    }
    switch (opcode) {
        case 0x08: printf("ADDI %s, %s, %d",       reg_name(t->id_rt), reg_name(t->id_rs), t->id_imm); return;
        case 0x23: printf("LW   %s, %d(%s)",        reg_name(t->id_rt), t->id_imm, reg_name(t->id_rs)); return;
        case 0x2B: printf("SW   %s, %d(%s)",        reg_name(t->id_rt), t->id_imm, reg_name(t->id_rs)); return;
        case 0x04: printf("BEQ  %s, %s, %d",        reg_name(t->id_rs), reg_name(t->id_rt), t->id_imm); return;
        default:   printf("opcode=0x%02X", opcode); return;
    }
}

void stages_reset(void) {
    memset(traces, 0, sizeof(traces));
    n_traces              = 0;
    if_fetch              = 0;
    if_stall              = 0;
    if_flush              = 0;
    ex_branch_taken_total = 0;
}

void stages_print_stats(int total_cycles, int total_stalls) {
    float ipc = total_cycles > 0 ? (float)n_traces / total_cycles : 0;

    printf("==========================================\n");
    printf("   RESULTADO DO PIPELINE MIPS\n");
    printf("==========================================\n\n");

    for (int i = 0; i < n_traces; i++) {
        InstrTrace *t = &traces[i];

        printf("------------------------------------------\n");
        printf(" Instrucao %d: 0x%08X  ", i + 1, t->inst);
        print_mnemonic(t);
        printf("\n");
        printf("------------------------------------------\n");

        // IF
        printf(" [IF ] Busca         : PC=0x%08X  inst=0x%08X\n",
               t->if_pc, t->inst);

        // ID
        printf(" [ID ] Decodificacao : %s=%u  %s=%u",
               reg_name(t->id_rs), t->id_data1,
               reg_name(t->id_rt), t->id_data2);
        if (t->id_ALUSrc)
            printf("  imm=%d", t->id_imm);
        printf("\n");
        printf("          Controle  : RegWrite=%u  ALUSrc=%u  MemRead=%u"
               "  MemWrite=%u  Branch=%u\n",
               t->id_RegWrite, t->id_ALUSrc,
               t->id_MemRead,  t->id_MemWrite, t->id_Branch);

        // EX
        printf(" [EX ] Execucao      : %u %c %u = %u  ->  dest=%s\n",
               t->ex_op1,
               t->id_MemWrite ? '+' : (t->id_opcode == 0 && (t->inst & 0x3F) == 0x22 ? '-' : '+'),
               t->ex_op2,
               t->ex_result,
               t->id_RegWrite ? reg_name(t->ex_dest) : "(sem reg)");
        if (t->ex_branch_taken)
            printf("          Branch tomado para 0x%08X\n",
                   t->if_pc + 4 + ((unsigned int)(t->id_imm) << 2));

        // MEM
        printf(" [MEM] Memoria       : ");
        if (t->mem_op == 1)
            printf("LW  mem[0x%X] -> %u\n", t->mem_addr, t->mem_rdata);
        else if (t->mem_op == 2)
            printf("SW  mem[0x%X] <- %u\n", t->mem_addr, t->mem_wdata);
        else
            printf("passthrough  (alu_result=%u)\n", t->ex_result);

        // WB
        printf(" [WB ] Write Back    : ");
        if (t->wb_written)
            printf("%s <- %u\n", reg_name(t->wb_reg), t->wb_value);
        else
            printf("(sem escrita)\n");

        printf("\n");
    }

    printf("==========================================\n");
    printf("   RESUMO GERAL\n");
    printf("==========================================\n");
    printf(" Ciclos totais        : %d\n",   total_cycles);
    printf(" Instrucoes reais     : %d\n",   n_traces);
    printf(" IPC                  : %.2f\n", ipc);
    printf(" Stalls (load-use)    : %d\n",   total_stalls);
    printf(" Flushes (branch)     : %d\n",   ex_branch_taken_total);
    printf(" Ciclos ociosos IF    : %d stall  %d flush\n\n", if_stall, if_flush);
}
