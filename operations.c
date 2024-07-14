#include "operations.h"
#include "opcode.h"
#include "process.h"
#include "value.h"

/*
There's no such thing as calling convention: arguments are simply specialised with the spec instruction (synonymous to apply); call is spec without dependencies to specialise
ISA:

Special:
o typeof $addr: r0 <- @($addr + 2)
o sizeof $addr: r0 <- @($addr + 1)
o at $dst, $addr: if r:idx >= $addr.size then outofbounds else $dst <- @($addr + r:idx) fi (* $addr must contain a slice *)
o atnochk $dst, $addr: $dst <- @($addr + r:idx)
o setat $addr, $val: $(addr + r:idx) <- @$val
init $addr, #typeid, $slc: $types[#typeid].init($slc)
offset $dst, $addr, offs: $dst <- @($addr + offs)
//member $dst, $addr, offs: $dst <- @($addr + offs) (* $addr must contain a value of a slice type *)
new $dst, #typeid
kll $addr
spec $dst, #typeid, $slc (* specialises the type #typeid with values located in slice $slc and stores new type id in $dst *) spec $dst, $addr, $slc (* specialises the value under $addr with values located in slice $slc and stores the obtained value in $dst *) apply $dst, #typeid, $slc apply $dst, $addr, $slc
typchk $addr, #typeid
kndchk $addr, kind
typcpy #dst_typeid, #src_typeid: $types[#dst_typeid] <- $types[$src_typeid]
typeq $lhs, $rhs: typeof $lhs == typeof $rhs
typeq lhs, rhs: typeof lhs == typeof rhs
typed #typeid asvardtmp litidx: r:tmp <- ptr [KND_ENUM_LITERAL, ..., ..., ..., litidx] (* along with typed #typeid asvartmp only instruction of asvar family *)

slctypoffs $dst, #typeid, offs: $dst <- $types[#typeid] + offs

complit $dst, #typeid, prevlitidx: r0 <- $dst; r1 <- prevlitidx; call $types[#typeid].formulaptr

Common:


Comparators:
isin litidx, $slc: if litidx in ($slc.elems)..($slc+$slc.size) then 1 else 0 fi
isinstmp litidx: if litidx in (slice(r:tmp).elems)..(r:tmp+slice(r:tmp).size) then 1 else 0 fi
between litidx: isinstmp litidx;
typed #typeid, isin litidx, $slc: for i in 1..$slc.size) do typchk $slc[i], #typeid; eq litidx, $slc[i]; if fl:eq then break fi done
typed #typeid between litidx: typed #typeid isinstmp litidx
eq $lhs, $rhs: fl:eq, fl:zr, r:pmres <- $lhs..($lhs.size) == $rhs..($rhs.size)
neq $lhs, $rhs: eq $lhs, $rhs; r:pmres <- not r:pmres
lt $lhs, $rhs
gt $lhs, $rhs
lte $lhs, $rhs
gte $lhs, $rhs
gaps
predgaps
regex
childof_1, #child, #parent: r:pmres <- $types[#child].parent_tid == #parent
childof_0, #child, #parent: r:pmres <- for it = $types[#child] to $types[#parent] do if it.parent_tid == 0 then return 0 elseif it.parent_tid == #parent then return 1 else it = $types[it.parent_tid]; continue fi done
childof n, #child, #parent: r:pmres <- it = $types[#child]; for i in 0..(n - 1) do it = $types[it.parent_tid] done; if it.parent_tid == #parent then 1 else 0 fi

Interop:
hostos
hostarch
syscall
in
out
breakpoint

Concurrency:
spawn
join
sync
*/

DTVM_Signal op_nop(dtvm_word* args)
{
    return DTVM_SUCCESS;
}
DTVM_Signal op_halt(dtvm_word* args)
{
    curr_halt = true;
    return DTVM_SUCCESS;
}
DTVM_Signal op_jmp(dtvm_word* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    if(args[0] >= MAX_MEMORY) return DTVM_NOADDR;
    //printf("jmp: args[0] = %ld\n", args[0]);
    curr_ip = args[0];
    return DTVM_SUCCESS;
}
DTVM_Signal op_jmpshort(dtvm_word* args)
{
    curr_ip += args[0];
    return DTVM_SUCCESS;
}
DTVM_Signal op_repeat(dtvm_word* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    if(args[0] >= MAX_MEMORY) return DTVM_NOADDR;
    for(size_t i = 0; i < args[1]; i++)
    {
        size_t last_ip = curr_ip;
        curr_ip = args[0];
        Operation op = dtvm_fetch_instr();
        dtvm_execute(op);
        curr_ip = last_ip;
    }
    return DTVM_SUCCESS;
}
DTVM_Signal op_call(dtvm_word* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    if(args[0] >= MAX_MEMORY) return DTVM_NOADDR;
    if(curr_csp >= MAX_CALL_DEPTH) return DTVM_CSOVFLOW;

    curr_call_stack[curr_csp++] = curr_ip;
    curr_ip = args[0];
    //printf("CALL: new ip: %ld\n", curr_ip);

    return DTVM_SUCCESS;
}
DTVM_Signal op_ret(dtvm_word* args)
{
    if(curr_csp == 0) return DTVM_CSUDFLOW;
    curr_ip = curr_call_stack[curr_csp-- - 1];
    return DTVM_SUCCESS;
}
DTVM_Signal op_throw(dtvm_word* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    if(args[0] >= MAX_MEMORY) return DTVM_NOADDR;
    dtvm_proc_throw(curr_proc, args[0]);
    return DTVM_SUCCESS;
}
DTVM_Signal op_catch(dtvm_word* args)
{
    if(curr_regs.sig == 0); // nothing has been thrown
    else if(curr_regs.sig != args[0])
    {
        curr_ip = args[1];
    }
    else
    {
        curr_sig_mode = false;
        curr_regs.sig = 0;
    }
    return DTVM_SUCCESS;
}
DTVM_Signal op_mov(dtvm_word* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    if(args[0] >= MAX_MEMORY) return DTVM_NOADDR;
    curr_ram[args[0]] = args[1];
    return DTVM_SUCCESS;
}
DTVM_Signal op_movregion(dtvm_word* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    if(args[0] >= MAX_MEMORY) return DTVM_NOADDR;
    if(curr_ram[args[0] + 1] < curr_ram[args[1] + 1]) return DTVM_NOSPACE;
    memmove(curr_ram + args[0], curr_ram + args[1], curr_ram[args[1] + 1]);
    return DTVM_SUCCESS;
}
DTVM_Signal op_cpy(dtvm_word* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    if(args[0] >= MAX_MEMORY || args[1] >= MAX_MEMORY) return DTVM_NOADDR;
    curr_ram[args[0]] = curr_ram[args[1]];
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldr0(dtvm_word* args)
{
    if(args[0] >= NREGS) return DTVM_NOREG;
    curr_regs.r0 = curr_ram[REG_STORE_BASE + REG_R0];
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldr1(dtvm_word* args)
{
    if(args[0] >= NREGS) return DTVM_NOREG;
    curr_regs.r1 = curr_ram[REG_STORE_BASE + REG_R1];
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldr2(dtvm_word* args)
{
    if(args[0] >= NREGS) return DTVM_NOREG;
    curr_regs.r2 = curr_ram[REG_STORE_BASE + REG_R2];
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldr3(dtvm_word* args)
{
    if(args[0] >= NREGS) return DTVM_NOREG;
    curr_regs.r3 = curr_ram[REG_STORE_BASE + REG_R3];
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldridx(dtvm_word* args)
{
    if(args[0] >= NREGS) return DTVM_NOREG;
    curr_regs.r3 = curr_ram[REG_STORE_BASE + REG_R3];
    return DTVM_SUCCESS;
}
DTVM_Signal op_str0(dtvm_word* args)
{
    if(args[0] >= NREGS) return DTVM_NOREG;
    curr_ram[REG_STORE_BASE + REG_R0] = curr_regs.r0;
    return DTVM_SUCCESS;
}
DTVM_Signal op_str1(dtvm_word* args)
{
    if(args[0] >= NREGS) return DTVM_NOREG;
    curr_ram[REG_STORE_BASE + REG_R1] = curr_regs.r1;
    return DTVM_SUCCESS;
}
DTVM_Signal op_str2(dtvm_word* args)
{
    if(args[0] >= NREGS) return DTVM_NOREG;
    curr_ram[REG_STORE_BASE + REG_R2] = curr_regs.r2;
    return DTVM_SUCCESS;
}
DTVM_Signal op_str3(dtvm_word* args)
{
    if(args[0] >= NREGS) return DTVM_NOREG;
    curr_ram[REG_STORE_BASE + REG_R3] = curr_regs.r3;
    return DTVM_SUCCESS;
}
DTVM_Signal op_stridx(dtvm_word* args)
{
    if(args[0] >= NREGS) return DTVM_NOREG;
    curr_ram[REG_STORE_BASE + REG_IDX] = curr_regs.idx;
    return DTVM_SUCCESS;
}
DTVM_Signal op_swp(dtvm_word* args)
{
    if(args[0] >= MAX_MEMORY || args[1] >= MAX_MEMORY) return DTVM_NOADDR;
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    curr_ram[args[0]] ^= curr_ram[args[1]] ^= curr_ram[args[0]] ^= curr_ram[args[1]];
    return DTVM_SUCCESS;
}
DTVM_Signal op_typeof(dtvm_word* args)
{
    if(args[0] + 2 >= MAX_MEMORY) return DTVM_NOADDR;
    if(args[0] == 0) return DTVM_PTR0;
    curr_regs.r0 = curr_ram[args[0] + 2];
    return DTVM_SUCCESS;
}
DTVM_Signal op_sizeof(dtvm_word* args)
{
    if(args[0] + 1 >= MAX_MEMORY) return DTVM_NOADDR;
    if(args[0] == 0) return DTVM_PTR0;
    curr_regs.r0 = curr_ram[args[0] + 1];
    return DTVM_SUCCESS;
}
DTVM_Signal op_at(dtvm_word* args)
{
    if(args[0] >= MAX_MEMORY || args[1] + curr_regs.idx >= MAX_MEMORY) return DTVM_NOADDR;
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;

    Value* val = (Value*)(curr_ram + args[0]);
    dtvm_word kind = val->kind;
    dtvm_word len;
    size_t at;
    if(kind == KND_SLICE)// || kind == KND_TYPE_SLICE)) return DTVM_ILLKND;
    {
        SliceLiteralValue slice = *(SliceLiteralValue*)(&val->as.slice_literal);
        len = slice.nelems;
        if(curr_regs.idx >= len) return DTVM_ILLIDX;
        at = args[0];
        for(size_t i = 0; i != curr_regs.idx; i++)
        {
            at += (*(Value*)(&curr_ram[at])).size;
        }
    }
    else if(kind == KND_TYPE_SLICE)
    {
        TypeSliceLiteralValue slice = *(TypeSliceLiteralValue*)(&val->as.type_slice_literal);
        len = slice.ntypes;
        if(curr_regs.idx >= len) return DTVM_ILLIDX;
        at = slice.typeids[curr_regs.idx];
    }
    else return DTVM_ILLKND;
    //memcpy(curr_ram + args[0], curr_ram[at], sizeof(dtvm_word) * curr_ram[at + 1]);
    curr_ram[args[0]] = at;
    return DTVM_SUCCESS;
}
DTVM_Signal op_atnochk(dtvm_word* args)
{
    if(args[0] >= MAX_MEMORY || args[1] + curr_regs.idx >= MAX_MEMORY) return DTVM_NOADDR;
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;

    Value* val = (Value*)(curr_ram + args[0]);
    dtvm_word kind = val->kind;
    dtvm_word len;
    size_t at;
    if(kind == KND_SLICE)// || kind == KND_TYPE_SLICE)) return DTVM_ILLKND;
    {
        SliceLiteralValue slice = *(SliceLiteralValue*)(&val->as.slice_literal);
        len = slice.nelems;
        at = args[0];
        for(size_t i = 0; i != curr_regs.idx; i++)
        {
            at += (*(Value*)(&curr_ram[at])).size;
        }
    }
    else if(kind == KND_TYPE_SLICE)
    {
        TypeSliceLiteralValue slice = *(TypeSliceLiteralValue*)(&val->as.type_slice_literal);
        len = slice.ntypes;
        at = slice.typeids[curr_regs.idx];
    }
    else return DTVM_ILLKND;
    //memcpy(curr_ram + args[0], curr_ram[at], sizeof(dtvm_word) * curr_ram[at + 1]);
    curr_ram[args[0]] = at;
    return DTVM_SUCCESS;
}
DTVM_Signal op_setat(dtvm_word* args)
{
    if(args[0] >= MAX_MEMORY || args[1] >= MAX_MEMORY) return DTVM_NOADDR;
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;

    Value* val = (Value*)(curr_ram + args[0]);
    dtvm_word kind = val->kind;
    dtvm_word len;
    size_t at;
    if(kind == KND_SLICE)// || kind == KND_TYPE_SLICE)) return DTVM_ILLKND;
    {
        SliceLiteralValue slice = *(SliceLiteralValue*)(&val->as.slice_literal);
        len = slice.nelems;
        if(curr_regs.idx >= len) return DTVM_ILLIDX;
        at = args[0];
        for(size_t i = 0; i != curr_regs.idx; i++)
        {
            at += (*(Value*)(&curr_ram[at])).size;
        }
    }
    else if(kind == KND_TYPE_SLICE)
    {
        TypeSliceLiteralValue slice = *(TypeSliceLiteralValue*)(&val->as.type_slice_literal);
        len = slice.ntypes;
        if(curr_regs.idx >= len) return DTVM_ILLIDX;
        at = slice.typeids[curr_regs.idx];
    }
    else return DTVM_ILLKND;
    //memcpy(curr_ram + args[0], curr_ram[at], sizeof(dtvm_word) * curr_ram[at + 1]);
    curr_ram[at] = curr_ram[args[1]];
    return DTVM_SUCCESS;
}
DTVM_Signal op_setatnochk(dtvm_word* args)
{
    if(args[0] >= MAX_MEMORY || args[1] >= MAX_MEMORY) return DTVM_NOADDR;
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;

    Value* val = (Value*)(curr_ram + args[0]);
    dtvm_word kind = val->kind;
    dtvm_word len;
    size_t at;
    if(kind == KND_SLICE)// || kind == KND_TYPE_SLICE)) return DTVM_ILLKND;
    {
        SliceLiteralValue slice = *(SliceLiteralValue*)(&val->as.slice_literal);
        len = slice.nelems;

        at = args[0];
        for(size_t i = 0; i != curr_regs.idx; i++)
        {
            at += (*(Value*)(&curr_ram[at])).size;
        }
    }
    else if(kind == KND_TYPE_SLICE)
    {
        TypeSliceLiteralValue slice = *(TypeSliceLiteralValue*)(&val->as.type_slice_literal);
        len = slice.ntypes;

        at = slice.typeids[curr_regs.idx];
    }
    else return DTVM_ILLKND;
    //memcpy(curr_ram + args[0], curr_ram[at], sizeof(dtvm_word) * curr_ram[at + 1]);
    curr_ram[at] = curr_ram[args[1]];
    return DTVM_SUCCESS;
}
// TODO: Can op_movregion be a good-enough low level replacement for this?

/* DTVM_Signal op_init(dtvm_word* args)
{
    if(args[0] >= MAX_MEMORY || args[2] >= MAX_MEMORY) return DTVM_NOADDR;
    if(args[0] == 0 || args[2] == 0) return DTVM_PTR0;

    *(TypeValue*)&curr_ram[TYPTBL + args[1]]
    return DTVM_SUCCESS;
} */
DTVM_Signal op_offset(dtvm_word* args)
{
    if(args[0] > MAX_MEMORY || args[1] >= MAX_MEMORY) return DTVM_NOADDR;
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    curr_ram[args[0]] = curr_ram[args[1] + args[2]];
    return DTVM_SUCCESS;
}

DTVM_Instruction opcodes[NOPCODES] = {
    op_nop, op_halt, op_jmp, op_jmpshort, op_repeat, op_call, op_ret, op_throw, op_catch, op_mov,
    op_movregion, op_cpy, op_ldr0, op_ldr1, op_ldr2, op_ldr3, op_str0, op_str1, op_str2, op_str3,
    op_ldridx, op_stridx, op_swp, op_typeof, op_sizeof, op_at, op_atnochk, op_setat, op_setatnochk
};
