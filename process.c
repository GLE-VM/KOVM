#include<stdio.h>
#include "process.h"
#include "operations.h"

Process* curr_proc;
dtvm_word curr_ip, curr_csp;
dtvm_byte curr_ram[MAX_MEMORY];
dtvm_word curr_call_stack[MAX_CALL_DEPTH];
Regs curr_regs;
PMRegs curr_pmregs;
Flags curr_flags;
bool curr_halt;
bool curr_sig_mode;
dtvm_word curr_call;
void dtvm_proc_load(Process* self, dtvm_byte* image, size_t img_len)
{
    //self->ram = make([]u64, len(image));
    memcpy(self->ram, image, img_len * sizeof(dtvm_byte));
    /*for(size_t i = 0; i < img_len; i++)
    {
        printf("ram[%ld] = %d\n", i, self->ram[i]);
    }*/
}


DTVM_Signal dtvm_proc_run(Process* self)
{
    curr_proc = self;
    curr_ip = 1;
    curr_halt = false;
    curr_sig_mode = false;
    curr_csp = 0;
    curr_call = curr_ip;
    memcpy(curr_ram, curr_proc->ram, sizeof(dtvm_byte) * MAX_MEMORY);

    DTVM_Signal result;
    while(!curr_halt)
    {
        struct timeval start, stop;
        gettimeofday(&start, 0);
        Opcode op = dtvm_fetch_instr();
        // puts("After fetch");
        if(curr_sig_mode && op != OP_CATCH)
            0;
        else
        {
            //printf("Instruction: %d\n", op);
            result = dtvm_execute(op);
            if(result) return result;
        }
        gettimeofday(&stop, 0);
        //printf("Time: %ld\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
        //puts("After execute");
    }
    return DTVM_SUCCESS;
}

void dtvm_proc_throw(Process* self, dtvm_word sig)
{
    self->sig_mode = true;
    self->regs.sig = sig;
}
