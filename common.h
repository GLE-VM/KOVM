#ifndef DTVM_COMMON_H
#define DTVM_COMMON_H

#include<stdint.h>
#include<stdbool.h>

typedef enum
{
    DTVM_SUCCESS,
    DTVM_PTR0,
    DTVM_CSUDFLOW,
    DTVM_CSOVFLOW,
    DTVM_NOADDR
} DTVM_Error;

const char* dtvm_get_error_msg(DTVM_Error err)
{
    switch(err)
    {
    case DTVM_SUCCESS: return "Success";
    case DTVM_PTR0:    return "Attempted to access value at address 0";
    case DTVM_CSUDFLOW:return "Call stack underflow";
    case DTVM_CSOVFLOW:return "Call stack overflow";
    case DTVM_NOADDR:  return "Attempted to access value at a non-existent address";
    }
}

typedef uint64_t dtvm_word;

#define MAX_MEMORY 16
#define MAX_CALL_DEPTH 512
#define MAX_PROCESSES 10
#define MAX_ARGS 4
#define NOPCODES 20
#define NREGS 7
#define NPMREGS 4

#define REG_IDX 0
#define REG_SIG 1
#define REG_R0  2
#define REG_R1  3
#define REG_R2  4
#define REG_R3  5
#define REG_TMP 6

#define REG_PM0 0
#define REG_PM1 1
#define REG_PM2 2
#define REG_PMR 3

typedef struct
{
    dtvm_word ip, csp;
    dtvm_word ram[MAX_MEMORY];
    dtvm_word call_stack[MAX_CALL_DEPTH];
    dtvm_word regs[NREGS];
    dtvm_word pmregs[NPMREGS];
    bool halt;
    bool sig_mode;
} Process;
Process* curr_proc;

typedef struct
{
    dtvm_word instr;
    uint8_t addr_modes[MAX_ARGS];
} Opcode;

typedef enum
{
    AMODE_NOARG, AMODE_LIT, AMODE_REG
} AddrMode;
#define ARITY_0 (AMODE_NOARG << 24) | (AMODE_NOARG << 16) | (AMODE_NOARG << 8) | AMODE_NOARG
#define ARITY_1 (AMODE_NOARG << 16) | (AMODE_NOARG <<  8) | AMODE_NOARG
#define ARITY_2 (AMODE_NOARG <<  8) | AMODE_NOARG
#define ARITY_3 AMODE_NOARG

typedef struct
{
    dtvm_word instr;
    dtvm_word args[MAX_ARGS];
} Operation;

typedef DTVM_Error (*Instruction)(dtvm_word* args);

Operation dtvm_fetch_instr();
DTVM_Error dtvm_execute(Operation op);
void dtvm_proc_throw(Process* self, dtvm_word sig);
#include "operations.h"

Instruction opcodes[NOPCODES] = {
    op_nop, op_halt, op_jmp, op_jmpshort, op_repeat, op_call, op_ret, op_throw, op_catch,
    op_mov, op_cpy
};

#endif