#ifndef DTVM_VALUE_H
#define DTVM_VALUE_H

#include<stdlib.h>

#include "common.h"


typedef enum
{
    KND_ENUM_LITERAL,
    KND_SLICE,
    KND_TYPE_SLICE
} Kind;


#define ValueCommon \
size_t ndeps; \
    dtvm_word deps;

typedef struct
{
    Kind kind;
    dtvm_word size;
    dtvm_word tid; // LSB of enum tid is the infinite? bit; for types it's irrelevant
    union
    {
        struct
        {
            ValueCommon
            dtvm_word ret;
            dtvm_word len;
            dtvm_word* code;
        } enum_literal;
        struct
        {
            ValueCommon
            dtvm_word nelems;
            struct Value* values;
        } slice_literal;
        struct
        {
            ValueCommon
            dtvm_word ntypes;
            dtvm_word* typeids;
        } type_slice_literal;
        struct
        {
            ValueCommon
            dtvm_word parent_tid;
        } type_value;
    } as;
} Value;


typedef struct
{
    ValueCommon
    dtvm_word ret;
    dtvm_word len;
    dtvm_word* code;
} EnumLiteralValue;
typedef struct
{
    ValueCommon
    dtvm_word nelems;
    Value* values;
} SliceLiteralValue;
typedef struct
{
    ValueCommon
    dtvm_word ntypes;
    dtvm_word* typeids;
} TypeSliceLiteralValue;


#endif