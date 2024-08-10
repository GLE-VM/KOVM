#ifndef DTVM_VALUE_H
#define DTVM_VALUE_H

#include<stdlib.h>
#include "common.h"

#define TPID 0
#define DEPS 4
#define SLIC 8
#define BLCK 12

#define VFIELD_FIRST 0xFFFFFFFF00000000

#define ByteToWordEndianess(field, val)\
dtvm_word field = ntohl(*(dtvm_word*)(val->field))

#define ByteToWordEndianessTemp(field, layout_type, baseaddr)\
dtvm_word field = ntohl(*(dtvm_word*)((layout_type*)(curr_ram + baseaddr))->field)
typedef struct
{
    dtvm_byte value[4*8];
} LiteralValue;

typedef struct
{
    dtvm_byte tid[4];
    dtvm_byte depsptr[4];  // ?
    dtvm_byte sliceptr[4];
    dtvm_byte blockptr[4];
} SliceValue;

typedef struct
{
    dtvm_byte tid[4];
    dtvm_byte depsptr[4];
    dtvm_byte blockptr[4];
    dtvm_byte padding[4];
} CellValue;

typedef struct
{
    dtvm_byte tid[4];
    dtvm_byte depsptr[4];
    dtvm_byte blockptr[4];
    dtvm_byte padding[4];
} TypeValue;

typedef struct
{
    dtvm_byte tid[4];
    dtvm_byte depsptr[4];
    dtvm_byte ptr[4];
    dtvm_byte padding[4];
} PtrValue;

// ???
typedef struct
{
    dtvm_byte ptr[4];
    dtvm_byte padding[4]; // MSB of [0] is 1 - host heap; 0 - virtual heap; next one is 1 - not null; 0 - null
} HeapObjectValue;

/*typedef struct
{
    dtvm_byte nodesz;
    dtvm_byte nextptr;
    dtvm_byte data[MAX_VHEAP_NODE_SIZE - 2];
} VirtualHeapNodeValue;*/

#endif
