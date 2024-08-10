#ifndef DTVM_COMMON_H
#define DTVM_COMMON_H

#include<stdint.h>
#include<stdbool.h>
#include<string.h>


typedef uint64_t dtvm_word;
typedef uint8_t dtvm_byte;

#define MAX_MEMORY 96
#define MAX_CALL_DEPTH 512
#define MAX_PROCESSES 10
#define MAX_ARGS 4
#define MAX_VHEAP_NODE_SIZE 32
#define NOPCODES 64
#define NREGS 7
#define NPMREGS 4


#endif
