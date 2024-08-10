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
    OP_RETN,
    OP_THROW,
    OP_CATCH,
    OP_MOV,
    OP_CPYREGION,
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
    OP_SWPREGION,
//    OP_TYPEOF,
//    OP_SIZEOF,
//    OP_NUMOF,
    OP_AT,
    OP_ATNOCHK,
    OP_SETAT,
    OP_SETATNOCHK,
    OP_OFFSET,
    //OP_INIT,
//    OP_SPEC,
//    OP_TYPCHK,
//    OP_TYPCHK_PEDANTIC
} Mnemonic;

DTVM_Signal op_nop(dtvm_byte* args);

DTVM_Signal op_halt(dtvm_byte* args);

DTVM_Signal op_jmp(dtvm_byte* args);

DTVM_Signal op_jmpshort(dtvm_byte* args);

DTVM_Signal op_repeat(dtvm_byte* args);

DTVM_Signal op_call(dtvm_byte* args);

DTVM_Signal op_ret(dtvm_byte* args);

DTVM_Signal op_throw(dtvm_byte* args);

DTVM_Signal op_catch(dtvm_byte* args);

DTVM_Signal op_mov(dtvm_byte* args);

DTVM_Signal op_cpyregion(dtvm_byte* args);

DTVM_Signal op_cpy(dtvm_byte* args);

DTVM_Signal op_ldr0(dtvm_byte* args);

DTVM_Signal op_ldr1(dtvm_byte* args);

DTVM_Signal op_ldr2(dtvm_byte* args);

DTVM_Signal op_ldr3(dtvm_byte* args);

DTVM_Signal op_ldridx(dtvm_byte* args);

DTVM_Signal op_str0(dtvm_byte* args);

DTVM_Signal op_str1(dtvm_byte* args);

DTVM_Signal op_str2(dtvm_byte* args);

DTVM_Signal op_str3(dtvm_byte* args);

DTVM_Signal op_stridx(dtvm_byte* args);

DTVM_Signal op_swp(dtvm_byte* args);

DTVM_Signal op_swpregion(dtvm_byte* args);
//DTVM_Signal op_typeof(dtvm_byte* args);

//DTVM_Signal op_sizeof(dtvm_byte* args);

//DTVM_Signal op_numof(dtvm_byte* args);

DTVM_Signal op_at(dtvm_byte* args);

DTVM_Signal op_atnochk(dtvm_byte* args);

DTVM_Signal op_setat(dtvm_byte* args);

DTVM_Signal op_setatnochk(dtvm_byte* args);

// TODO: Can op_movregion be a good-enough low level replacement for this?

DTVM_Signal op_offset(dtvm_byte* args);

//DTVM_Signal op_spec(dtvm_byte* args);

typedef DTVM_Signal (*DTVM_Instruction)(dtvm_byte*);
extern DTVM_Instruction opcodes[NOPCODES];


#endif
