#ifndef DTVM_PROCESS_H
#define DTVM_PROCESS_H

#include<stdlib.h>
#include<string.h>

#include "common.h"
#include "opcode.h"


void dtvm_proc_load(Process* self, dtvm_word* image, size_t img_len)
{
    //self->ram = make([]u64, len(image));
    memcpy(self->ram, image, img_len * sizeof(dtvm_word));
    for(size_t i = 0; i < img_len; i++)
    {
        printf("ram[%ld] = %ld\n", i, self->ram[i]);
    }
}

DTVM_Error dtvm_proc_run(Process* self)
{
    curr_proc = self;
    curr_proc->ip = 1;
    curr_proc->halt = false;
    DTVM_Error result;
    while(!curr_proc->halt)
    {
        Operation op = dtvm_fetch_instr();
        puts("After fetch");
        if(curr_proc->sig_mode && op.instr != OP_CATCH)
            0;
        else
        {
            printf("Instruction: %ld\n", op.instr);
            result = dtvm_execute(op);
            if(result) return result;
        }
        puts("After execute");
    }
    return DTVM_SUCCESS;
}

void dtvm_proc_throw(Process* self, dtvm_word sig)
{
    self->sig_mode = true;
    self->regs[REG_SIG] = sig;
}

#endif