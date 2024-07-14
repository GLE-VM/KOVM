#include "vm.h"

void dtvm_init(DTVM* self, dtvm_word nprocs)
{
    //self->processes = make([dynamic]Process, nprocs);
    self->next_idx = 0;
}

dtvm_word dtvm_load(DTVM* self, dtvm_word* image, size_t img_len)
{
    self->next_idx++;
    if(MAX_PROCESSES <= self->next_idx)
    {
        return 0;
    }
    dtvm_proc_load(&(self->processes[self->next_idx - 1]), image, img_len);

    return self->next_idx;
}

void dtvm_unload(DTVM* self, dtvm_word idx)
{
    // ...
}

DTVM_Signal dtvm_run(DTVM* self, dtvm_word idx)
{
    return dtvm_proc_run(&(self->processes[idx - 1]));
}

void dtvm_destroy(DTVM* self)
{
    // ...
}
