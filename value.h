#ifndef DTVM_VALUE_H
#define DTVM_VALUE_H

#include<stdlib.h>

#include "common.h"

typedef enum
{
    ENUM_LITERAL
} Kind;

#define ValueCommon \
size_t ndeps; \
    dtvm_word deps;

typedef struct
{
    ValueCommon
    dtvm_word tid; // LSB of enum tid is the infinite? bit
    dtvm_word ret;
    dtvm_word len;
    dtvm_word* code;
} EnumLiteralValue;

typedef union
{
    EnumLiteralValue enum_literal;
} As;

typedef struct
{
    Kind kind;
    dtvm_word size;
    As as;
} Value;

#endif