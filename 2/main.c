#include <stdbool.h>
#include<stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCS 1
#define MAX_RAM   1024
#define MAX_CSTACK 1024
#define NREGS 9
#define NPMREGS 5
#define MAX_OPANDS 4

#define VM_SIG_RESERVED 0

#define FL_EQ (1 << 0)
#define FL_GT (1 << 1)
#define FL_LT (1 << 2)
#define FL_ZR (1 << 3)
#define FL_OVF (1 << 4)
#define FL_CHP (1 << 5)
#define FL_HALT (1 << 6)
#define FL_SIGMODE (1 << 7)

#define R_0 0
#define R_1 1
#define R_2 2
#define R_3 3
#define R_RES 4
#define R_IDX 5
#define R_SIG 6
#define R_TMP 7
#define R_BYTES 8

#define PM_0 0
#define PM_1 1
#define PM_2 2
#define PM_3 3
#define PM_RES 4

typedef uint8_t vm_byte;
typedef uint64_t vm_word;
typedef struct
{
    vm_word ip, csp;
    vm_byte* ram;
    vm_word* callstack;
    vm_word regs[NREGS]; // r0, r1, r2, r3, res, idx, sig, tmp
    vm_word pmregs[NPMREGS]; // pm0, pm1, pm2, pm3, pmres
    vm_byte flags;     // eq, gt, lt, zr, ovf, chp, halt, sigmode
    // vm_word curr_call;
} VM_Process;
VM_Process* curr_proc;
vm_word curr_ip, curr_csp;
vm_byte* curr_ram;
vm_word* curr_callstack;
vm_word curr_regs[NREGS];
vm_word curr_pmregs[NPMREGS];
vm_byte curr_flags;
// vm_word curr_curr_call;

VM_Process processes[MAX_PROCS];
vm_word next_proc_idx = 0;


void vm_exec_at(VM_Process* curr, vm_word ip);
void vm_throw_signal(vm_word sigaddr);
bool vm_reserved_addr(vm_word addr);


#define OPAND(idx) curr_ram[curr_ip + idx + 1]
void op_nop()
{
    return;
}
void op_halt()
{
    curr_flags |= FL_HALT;
}
void op_jmpa()
{
    curr_ip = OPAND(0);
}
void op_jmpshorto()
{
    curr_ip += OPAND(0);
}
void op_repeatai()
{
    for(vm_word i = 0; i < OPAND(1); i++)
    {
        vm_exec_at(curr_proc, (vm_word)OPAND(0));
    }
}
void op_calla()
{
    curr_callstack[curr_csp++] = curr_ip + 5;
    curr_ip = OPAND(0);
}
void op_ret()
{
    curr_ip = curr_callstack[--curr_csp];
}
void op_retni()
{
    curr_ip = curr_callstack[curr_csp -= OPAND(0)];
}
void op_throwa()
{
    vm_throw_signal(OPAND(0));
}
void op_acatcha()
{
    if(curr_regs[R_SIG] == 0) return;
    else if(curr_regs[R_SIG] != OPAND(0))
        curr_ip = OPAND(1);
    else
    {
        curr_flags &= ~FL_SIGMODE;
        curr_regs[R_SIG] = 0;
    }
}

void op_amovi()
{
    if(vm_reserved_addr(OPAND(0))) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(0)] = OPAND(1);
}
void op_amova()
{
    if(vm_reserved_addr(OPAND(0)) || vm_reserved_addr(OPAND(1))) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(0)] = curr_ram[OPAND(1)];
}
void op_slcmovslc()
{
    if(vm_reserved_addr(OPAND(0)) || vm_reserved_addr(OPAND(1))) vm_throw_signal(VM_SIG_RESERVED);
    memmove(curr_ram + OPAND(0), curr_ram + OPAND(1), OPAND(2));
}

void op_ldri()
{
    curr_regs[OPAND(0)] = OPAND(1);
}
void op_ldra()
{
    if(vm_reserved_addr(OPAND(1))) vm_throw_signal(VM_SIG_RESERVED);
    curr_regs[OPAND(0)] = curr_ram[OPAND(1)];
}
void op_stra()
{
    if(vm_reserved_addr(OPAND(0))) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(0)] = curr_regs[OPAND(1)];
}

void op_ldpmri()
{
    curr_pmregs[OPAND(0)] = OPAND(1);
}
void op_ldpmra()
{
    if(vm_reserved_addr(OPAND(1))) vm_throw_signal(VM_SIG_RESERVED);
    curr_pmregs[OPAND(0)] = curr_ram[OPAND(1)];
}
void op_stpmra()
{
    if(vm_reserved_addr(OPAND(0))) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(0)] = curr_pmregs[OPAND(1)];
}
void op_aswpa()
{
    if(vm_reserved_addr(OPAND(0)) || vm_reserved_addr(OPAND(1))) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(0)] ^= curr_ram[OPAND(1)] ^= curr_ram[OPAND(0)] ^= curr_ram[OPAND(1)];
}
void op_slcswpslc()
{
    if(vm_reserved_addr(OPAND(0)) || vm_reserved_addr(OPAND(1))) vm_throw_signal(VM_SIG_RESERVED);
    vm_byte* tmp = alloca(OPAND(2));
    memcpy(tmp, curr_ram + OPAND(1), OPAND(2));
    vm_byte* tmp_dst = curr_ram + OPAND(0);
    for(vm_word i = OPAND(0); i < OPAND(2); i++)
    {
        tmp_dst[i] ^= tmp[i] ^= tmp_dst[i] ^= tmp[i];
    }
}

void op_aatao()
{
    if(vm_reserved_addr(OPAND(1)) || vm_reserved_addr(OPAND(2))|| vm_reserved_addr(OPAND(2) + OPAND(0) * OPAND(3))) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(1)] = curr_ram[OPAND(2) + OPAND(0) * OPAND(3)];
}
void op_aataidx()
{
    if(vm_reserved_addr(OPAND(1)) || vm_reserved_addr(OPAND(2))|| vm_reserved_addr(OPAND(2) + OPAND(0) * curr_regs[R_IDX])) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(1)] = curr_ram[OPAND(2) + OPAND(0) * curr_regs[R_IDX]];
}
void op_aosetati()
{
    if(vm_reserved_addr(OPAND(1)) || vm_reserved_addr(OPAND(1) + OPAND(0) * OPAND(2))) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(1) + OPAND(0) * OPAND(2)] = OPAND(3);
}
void op_aosetata()
{
    if(vm_reserved_addr(OPAND(1)) || vm_reserved_addr(OPAND(3))|| vm_reserved_addr(OPAND(1) + OPAND(0) * OPAND(2))) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(1) + OPAND(0) * OPAND(2)] = curr_ram[OPAND(3)];
}
void op_aidxsetati()
{
    if(vm_reserved_addr(OPAND(1)) || vm_reserved_addr(OPAND(1) + OPAND(0) * curr_regs[R_IDX])) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(1) + OPAND(0) * curr_regs[R_IDX]] = OPAND(3);
}
void op_aidxsetata()
{
    if(vm_reserved_addr(OPAND(1)) || vm_reserved_addr(OPAND(3))|| vm_reserved_addr(OPAND(1) + OPAND(0) * curr_regs[R_IDX])) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(1) + OPAND(0) * curr_regs[R_IDX]] = curr_ram[OPAND(3)];
}

void op_aoffsetao()
{
    if(vm_reserved_addr(OPAND(0))) vm_throw_signal(VM_SIG_RESERVED);
    curr_ram[OPAND(0)] = curr_ram[OPAND(1) + OPAND(2)];
}
#undef OPAND
typedef void (*vm_instruction)();

vm_instruction dispatch[] =
{
    op_nop, op_halt, op_jmpa, op_jmpshorto, op_repeatai, op_calla, op_ret, op_retni, op_throwa, op_acatcha,
    op_amovi, op_amova, op_slcmovslc,
    op_ldri, op_ldra, op_stra, op_ldpmri, op_ldpmra, op_stpmra,
    op_aswpa, op_slcswpslc,
    op_aatao, op_aataidx, op_aosetati, op_aosetata, op_aidxsetati,
    op_aidxsetata,
    op_aoffsetao
};

#define OP_NOP 0
#define OP_HALT 1
#define OP_JMPA 2
#define OP_JMPSHORTO 3
#define OP_REPEATAI 4
#define OP_CALLA 5
#define OP_RET 6
#define OP_RETNI 7
#define OP_THROWA 8
#define OP_ACATCHA 9
#define OP_AMOVI 10
#define OP_AMOVA 11
#define OP_SLCMOVSLC 12
#define OP_LDRI 13
#define OP_LDRA 14
#define OP_STRA 15
#define OP_LDPMRI 16
#define OP_LDMPRA 17
#define OP_STPMRA 18
#define OP_ASWPA 19
#define OPSLCSWPSLC 20
#define OP_AATAO 21
#define OP_AATAIDX 22
#define OP_AOSETATI 23
#define OP_AOSETATA 24
#define OP_AIDXSETATI 25
#define OP_AIDXSETATA 26
#define OP_AOFFSETAO 27

bool vm_reserved_addr(vm_word addr)
{
    return false;
}
void vm_throw_signal(vm_word sigaddr)
{
    curr_flags |= FL_SIGMODE;
    curr_regs[R_SIG] = sigaddr;
}
void vm_exec_at(VM_Process* curr, vm_word ip)
{
    dispatch[curr->ram[ip]]();
}
void vm_exec_next()
{
    vm_word lastip = curr_ip;
    printf("Instruction: %d\n", curr_ram[curr_ip]);
    if(curr_ram[curr_ip] == OP_HALT || curr_ram[curr_ip] == OP_RET || curr_ram[curr_ip] == OP_RETNI || curr_ram[curr_ip] == OP_ACATCHA || !(curr_flags & FL_SIGMODE))
        dispatch[curr_ram[curr_ip]]();
    if(curr_ip == lastip) curr_ip += 5;
}
vm_word vm_process(vm_byte* img, vm_word imgln)
{
    processes[next_proc_idx].ram = alloca(MAX_RAM);
    explicit_bzero(processes[next_proc_idx].ram, MAX_RAM);
    processes[next_proc_idx].callstack = alloca(MAX_CSTACK);
    vm_byte* ram = processes[next_proc_idx].ram;
    const char* perm_denied = "Permission denied\0\0\0";
    memcpy(ram, perm_denied, 20);
    for(vm_word i = 0; (i < imgln) && (i < MAX_RAM); i++)
    {
        ram[i + 20] = img[i];
    }
    processes[next_proc_idx].ip = 20;
    return next_proc_idx++;
}
vm_word vm_run(vm_word pidx)
{
    curr_proc = &processes[pidx];
    curr_ram = curr_proc->ram;
    curr_callstack = curr_proc->callstack;
    curr_ip = curr_proc->ip;
    curr_csp = curr_proc->csp;
    curr_flags = curr_proc->flags;
    memcpy(curr_regs, curr_proc->regs, sizeof(vm_word) * NREGS);
    memcpy(curr_pmregs, curr_proc->pmregs, sizeof(vm_word) * NPMREGS);
    //printf("Successfully initiated execution of proc %ld\n", pidx);
    while(!(curr_flags & FL_HALT))
    {
        //printf("Executing\n");
        vm_exec_next();
    }
    return curr_regs[R_SIG];
}
int main(int argc, char** argv)
{
    vm_byte testimg[] =
    {
        OP_NOP, 0, 0, 0, 0,
        OP_JMPA, 30, 0, 0, 0,
        OP_CALLA, 45, 0, 0, 0,
        OP_THROWA, 50, 0, 0, 0,
        OP_HALT, 0, 0, 0, 0,
        OP_RET, 0, 0, 0, 0,
        'E', 'r', 'r', 'o', 'r', '\0'
    };
    vm_word pidx = vm_process(testimg, 35);
    vm_word sig = vm_run(pidx);
    char sigmsg[1000] = {};
    if(sig)
        //snprintf(sigmsg, processes[pidx].ram[sig], "%s", processes[pidx].ram + 1);
        printf("(%ld) %s\n", sig, processes[pidx].ram + sig);

    return 0;
}

