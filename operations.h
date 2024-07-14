#ifndef DTVM_OPERATIONS_H
#define DTVM_OPERATIONS_H

#include "common.h"
#include "signals.h"

typedef enum
{
    OP_NOP,
    OP_HALT,
    OP_JMP,
    OP_JMPSHORT,
    OP_REPEAT,
    OP_CALL,
    OP_RET,
    OP_THROW,
    OP_CATCH,
    OP_MOV,
    OP_MOVREGION,
    OP_CPY,
    OP_LDR0,
    OP_LDR1,
    OP_LDR2,
    OP_LDR3,
    OP_LDRIDX,
    OP_STR0,
    OP_STR1,
    OP_STR2,
    OP_STR3,
    OP_STRIDX,
    OP_SWP,
    OP_TYPEOF,
    OP_SIZEOF,
    OP_AT,
    OP_ATNOCHK,
    OP_SETAT,
    OP_SETATNOCHK,
    OP_OFFSET
    //OP_INIT
} Mnemonic;

DTVM_Signal op_nop(dtvm_word* args);

DTVM_Signal op_halt(dtvm_word* args);

DTVM_Signal op_jmp(dtvm_word* args);

DTVM_Signal op_jmpshort(dtvm_word* args);

DTVM_Signal op_repeat(dtvm_word* args);

DTVM_Signal op_call(dtvm_word* args);

DTVM_Signal op_ret(dtvm_word* args);

DTVM_Signal op_throw(dtvm_word* args);

DTVM_Signal op_catch(dtvm_word* args);

DTVM_Signal op_mov(dtvm_word* args);

DTVM_Signal op_movregion(dtvm_word* args);

DTVM_Signal op_cpy(dtvm_word* args);

DTVM_Signal op_ldr0(dtvm_word* args);

DTVM_Signal op_ldr1(dtvm_word* args);

DTVM_Signal op_ldr2(dtvm_word* args);

DTVM_Signal op_ldr3(dtvm_word* args);

DTVM_Signal op_ldridx(dtvm_word* args);

DTVM_Signal op_str0(dtvm_word* args);

DTVM_Signal op_str1(dtvm_word* args);

DTVM_Signal op_str2(dtvm_word* args);

DTVM_Signal op_str3(dtvm_word* args);

DTVM_Signal op_stridx(dtvm_word* args);

DTVM_Signal op_swp(dtvm_word* args);

DTVM_Signal op_typeof(dtvm_word* args);

DTVM_Signal op_sizeof(dtvm_word* args);

DTVM_Signal op_at(dtvm_word* args);

DTVM_Signal op_atnochk(dtvm_word* args);

DTVM_Signal op_setat(dtvm_word* args);

DTVM_Signal op_setatnochk(dtvm_word* args);

// TODO: Can op_movregion be a good-enough low level replacement for this?

DTVM_Signal op_offset(dtvm_word* args);

typedef DTVM_Signal (*DTVM_Instruction)(dtvm_word*);
extern DTVM_Instruction opcodes[NOPCODES];


#endif
