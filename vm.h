#ifndef DTVM_VM_H
#define DTVM_VM_H

#include "common.h"
#include "process.h"

typedef struct
{
    Process processes[MAX_PROCESSES];
    dtvm_word next_idx;
} DTVM;

void dtvm_init(DTVM* self, dtvm_word nprocs);

dtvm_word dtvm_load(DTVM* self, dtvm_word* image, size_t img_len);

void dtvm_unload(DTVM* self, dtvm_word idx);

DTVM_Signal dtvm_run(DTVM* self, dtvm_word idx);


void dtvm_destroy(DTVM* self);

#endif
