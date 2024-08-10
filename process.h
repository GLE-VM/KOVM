#ifndef DTVM_PROCESS_H
#define DTVM_PROCESS_H

#include<stdlib.h>
#include<string.h>
#include<sys/time.h>

#include "common.h"
#include "opcode.h"

typedef struct
{
    dtvm_word r0, r1, r2, r3;
    dtvm_word res;
    dtvm_word idx, sig, tmp;
} Regs;

typedef struct
{
    dtvm_word pm0, pm1, pm2, pm3, pmres;
} PMRegs;

typedef struct
{
    bool eq, gt, lt, zr, ovf, chp;
} Flags;

#define REG_R0  0
#define REG_R1  1
#define REG_R2  2
#define REG_R3  3
#define REG_RES 4
#define REG_IDX 5
#define REG_SIG 6
#define REG_TMP 7

#define REG_PM0 0
#define REG_PM1 1
#define REG_PM2 2
#define REG_PMR 3

#define FL_EQ  0
#define FL_GT  1
#define FL_LT  2
#define FL_ZR  3
#define FL_OVF 4
#define FL_CHP 5

#define REG_STORE_BASE (MAX_MEMORY - 11)
#define PMREG_STORE_OFFSET NREGS
#define TYPTBL 0

typedef struct
{
    dtvm_word ip, csp;
    dtvm_byte ram[MAX_MEMORY];
    dtvm_word call_stack[MAX_CALL_DEPTH];
    Regs regs;
    PMRegs pmregs;
    Flags flags;
    bool halt;
    bool sig_mode;
    dtvm_word curr_call;
} Process;

extern Process* curr_proc;
extern dtvm_word curr_ip, curr_csp;
extern dtvm_byte curr_ram[MAX_MEMORY];
extern dtvm_word curr_call_stack[MAX_CALL_DEPTH];
extern Regs curr_regs;
extern PMRegs curr_pmregs;
extern Flags curr_flags;
extern bool curr_halt;
extern bool curr_sig_mode;
extern dtvm_word curr_call;

void dtvm_proc_load(Process* self, dtvm_byte* image, size_t img_len);

DTVM_Signal dtvm_proc_run(Process* self);

void dtvm_proc_throw(Process* self, dtvm_word sig);

#endif
