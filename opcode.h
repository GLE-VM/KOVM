#ifndef DTVM_OPCODE_H
#define DTVM_OPCODE_H

#include<stdlib.h>
#include "common.h"
#include "signals.h"


typedef dtvm_byte Opcode;

typedef enum
{
    AMODE_NOARG, AMODE_LIT, AMODE_IDX, AMODE_DEP
} AddrMode;
#define ARITY_0 (AMODE_NOARG << 24) | (AMODE_NOARG << 16) | (AMODE_NOARG << 8) | AMODE_NOARG
#define ARITY_1 (AMODE_NOARG << 16) | (AMODE_NOARG <<  8) | AMODE_NOARG
#define ARITY_2 (AMODE_NOARG <<  8) | AMODE_NOARG
#define ARITY_3 AMODE_NOARG



dtvm_word dtvm_decode_arg(Opcode* const opcode, uint16_t nth);

Opcode dtvm_fetch_instr();

DTVM_Signal dtvm_execute(Opcode op);

#endif
