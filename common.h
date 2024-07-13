#ifndef DTVM_COMMON_H
#define DTVM_COMMON_H

#include<stdint.h>
#include<stdbool.h>
#include<string.h>


typedef enum
{
    DTVM_SUCCESS,
    DTVM_PTR0,
    DTVM_CSUDFLOW,
    DTVM_CSOVFLOW,
    DTVM_NOADDR,
    DTVM_NOREG,
    DTVM_ILLKND,
    DTVM_ILLIDX,
    DTVM_NOSPACE
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
    case DTVM_NOREG:   return "Attempted to access non-existent register";
    case DTVM_ILLKND:  return "Wrong kind";
    case DTVM_ILLIDX:  return "Specified index is out of bounds";
    case DTVM_NOSPACE: return "Not enough space";
    }
}

typedef uint64_t dtvm_word;

#define MAX_MEMORY 96
#define MAX_CALL_DEPTH 512
#define MAX_PROCESSES 10
#define MAX_ARGS 4
#define NOPCODES 32
#define NREGS 7
#define NPMREGS 4

#include "value.h"

typedef struct
{
    dtvm_word r0, r1, r2, r3;
    dtvm_word idx, sig, tmp;
} Regs;
typedef struct
{
    dtvm_word pm0, pm1, pm2, pmres;
} PMRegs;

#define REG_R0  2
#define REG_R1  3
#define REG_R2  4
#define REG_R3  5
#define REG_IDX 0
#define REG_SIG 1
#define REG_TMP 6

#define REG_PM0 0
#define REG_PM1 1
#define REG_PM2 2
#define REG_PMR 3

#define REG_STORE_BASE (MAX_MEMORY - 11)
#define PMREG_STORE_OFFSET NREGS
#define TYPTBL 0

typedef struct
{
    dtvm_word ip, csp;
    dtvm_word ram[MAX_MEMORY];
    dtvm_word call_stack[MAX_CALL_DEPTH];
    Regs regs;
    PMRegs pmregs;
    bool halt;
    bool sig_mode;
} Process;
Process* curr_proc;
dtvm_word curr_ip, curr_csp;
dtvm_word curr_ram[MAX_MEMORY];
dtvm_word curr_call_stack[MAX_CALL_DEPTH];
Regs curr_regs;
PMRegs curr_pmregs;
bool curr_halt;
bool curr_sig_mode;

typedef struct
{
    dtvm_word instr;
    uint8_t addr_modes[MAX_ARGS];
} Opcode;

typedef enum
{
    AMODE_NOARG, AMODE_LIT, AMODE_IDX
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
    op_nop, op_halt, op_jmp, op_jmpshort, op_repeat, op_call, op_ret, op_throw, op_catch, op_mov,
    op_movregion, op_cpy, op_ldr0, op_ldr1, op_ldr2, op_ldr3, op_str0, op_str1, op_str2, op_str3,
    op_ldridx, op_stridx, op_swp, op_typeof, op_sizeof, op_at, op_atnochk, op_setat, op_setatnochk
};

#endif