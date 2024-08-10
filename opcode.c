#include "opcode.h"
#include "process.h"
#include "operations.h"
#include<stdio.h>
    /*switch(opcode->addr_modes[nth])
    {
    case AMODE_LIT: return val;
    case AMODE_IDX: return val + curr_regs.idx;
    case AMODE_DEP: return curr_ram[curr_call + val];
    }*/
const size_t opcode_appetite = sizeof(Opcode);

Opcode dtvm_fetch_instr()
{
    Opcode opcode = *(Opcode*)(curr_ram + curr_ip);
    curr_ip += opcode_appetite;
    return opcode;
}

DTVM_Signal dtvm_execute(Opcode op)
{
    return opcodes[op](curr_ram + curr_ip);
}


