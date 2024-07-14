#ifndef DTVM_OPCODE_H
#define DTVM_OPCODE_H

#include<stdlib.h>
#include "common.h"
#include "signals.h"


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


dtvm_word dtvm_decode_arg(Opcode* const opcode, uint16_t nth);

Operation dtvm_fetch_instr();

DTVM_Signal dtvm_execute(Operation op);

#endif
