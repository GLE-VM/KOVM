#include<stdio.h>
#include<netinet/in.h>
#include "operations.h"
#include "common.h"
#include "opcode.h"
#include "process.h"
#include "signals.h"
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
x init $addr, #typeid, $slc: $types[#typeid].init($slc)
o offset $dst, $addr, offs: $dst <- @($addr + offs)
//member $dst, $addr, offs: $dst <- @($addr + offs) (* $addr must contain a value of a slice type *)
new $dst, #typeid
kll $addr
o spec $dst, #typeid, $slc (* specialises the type #typeid with values located in slice $slc and stores new type id in $dst *)
o spec $dst, $addr, $slc (* specialises the value under $addr with values located in slice $slc and stores the obtained value in $dst *)
o apply $dst, #typeid, $slc
o apply $dst, $addr, $slc
o typchk $addr, #typeid
o kndchk $addr, kind
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
ischild
*/

DTVM_Signal op_nop(dtvm_byte* args)
{
    return DTVM_SUCCESS;
}
DTVM_Signal op_halt(dtvm_byte* args)
{
    curr_halt = true;
    return DTVM_SUCCESS;
}
DTVM_Signal op_jmp(dtvm_byte* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    //printf("jmp: args[0] = %ld\n", args[0]);
    curr_ip = args[0];
    return DTVM_SUCCESS;
}
DTVM_Signal op_jmpshort(dtvm_byte* args)
{
    curr_ip += args[0];
    return DTVM_SUCCESS;
}
DTVM_Signal op_repeat(dtvm_byte* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    for(size_t i = 0; i < args[1]; i++)
    {
        size_t last_ip = curr_ip;
        curr_ip = args[0];
        Opcode op = dtvm_fetch_instr();
        dtvm_execute(op);
        curr_ip = last_ip;
    }
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_call(dtvm_byte* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    if(curr_csp >= MAX_CALL_DEPTH) return DTVM_CSOVFLOW;

    curr_call_stack[curr_csp++] = curr_ip + 1;
    curr_ip = args[0];
    //printf("CALL: new ip: %ld\n", curr_ip);

    return DTVM_SUCCESS;
}
DTVM_Signal op_ret(dtvm_byte* args)
{
    if(curr_csp == 0) return DTVM_CSUDFLOW;
    curr_ip = curr_call_stack[curr_csp-- - 1];
    return DTVM_SUCCESS;
}
DTVM_Signal op_retn(dtvm_byte* args)
{
    if(curr_csp < args[0]) return DTVM_CSUDFLOW;
    curr_ip = curr_call_stack[curr_csp - 1];
    curr_csp -= args[0];
    return DTVM_SUCCESS;
}
DTVM_Signal op_throw(dtvm_byte* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    dtvm_proc_throw(curr_proc, args[0]);
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_catch(dtvm_byte* args)
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
    curr_ip += 2;
    return DTVM_SUCCESS;
}
DTVM_Signal op_mov(dtvm_byte* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    curr_ram[args[0]] = args[1];
    curr_ip += 2;
    return DTVM_SUCCESS;
}
// TODO: Make the remaining space a region (i.e. assign its size to the offset 1)
DTVM_Signal op_cpyregion(dtvm_byte* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    if(args[2] == 0) return DTVM_NOSPACE;
    //if(curr_ram[args[0] + 1] < curr_ram[args[1] + 1]) return DTVM_NOSPACE;
    memmove(curr_ram + args[0], curr_ram + args[1], args[2]);//curr_ram[args[1] + 1]);
    curr_ip += 3;
    return DTVM_SUCCESS;
}
DTVM_Signal op_cpy(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    curr_ram[args[0]] = curr_ram[args[1]];
    curr_ip += 2;
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldr0(dtvm_byte* args)
{
    curr_regs.r0 = curr_ram[args[0]];
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldr1(dtvm_byte* args)
{
    curr_regs.r1 = curr_ram[args[0]];
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldr2(dtvm_byte* args)
{
    curr_regs.r2 = curr_ram[args[0]];
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldr3(dtvm_byte* args)
{
    curr_regs.r3 = curr_ram[args[0]];
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldrres(dtvm_byte* args)
{
    curr_regs.res = curr_ram[args[0]];
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_ldridx(dtvm_byte* args)
{
    curr_regs.r3 = curr_ram[args[0]];
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_str0(dtvm_byte* args)
{
    curr_ram[args[0]] = curr_regs.r0;
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_str1(dtvm_byte* args)
{
    curr_ram[args[0]] = curr_regs.r1;
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_str2(dtvm_byte* args)
{
    curr_ram[args[0]] = curr_regs.r2;
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_str3(dtvm_byte* args)
{
    curr_ram[args[0]] = curr_regs.r3;
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_strres(dtvm_byte* args)
{
    curr_ram[args[0]] = curr_regs.res;
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_stridx(dtvm_byte* args)
{
    curr_ram[args[0]] = curr_regs.idx;
    curr_ip++;
    return DTVM_SUCCESS;
}
DTVM_Signal op_swp(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    // NOTE: Hacker's swap algorithm in case you're curious
    curr_ram[args[0]] ^= curr_ram[args[1]] ^= curr_ram[args[0]] ^= curr_ram[args[1]];
    curr_ip += 2;
    return DTVM_SUCCESS;
}
DTVM_Signal op_swpregion(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    if(args[2] == 0) return DTVM_NOSPACE;
    // NOTE: Compatibility with previous-C-standards-conforming compilers
    dtvm_byte* tmp = alloca(sizeof(dtvm_word) * args[2]);

    // NOTE: Proceed with caution for it's been written just like that.
    memcpy(tmp, curr_ram + args[1], args[2]);

    dtvm_byte* tmp_dst = curr_ram + args[0];
    for(size_t i = sizeof(dtvm_word) * args[0]; i < sizeof(dtvm_word) * args[2]; i++)
    {
        *(tmp_dst + i) ^= tmp[i] ^= *(tmp_dst + i) ^= tmp[i];
    }
    curr_ip += 3;
    return DTVM_SUCCESS;
}
/*DTVM_Signal op_typeof(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    curr_ram[args[0]] = curr_ram[args[1] + 2];
    return DTVM_SUCCESS;
}
DTVM_Signal op_sizeof(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    curr_ram[args[0]] = curr_ram[args[1] + 1];
    return DTVM_SUCCESS;
}
DTVM_Signal op_numof(dtvm_byte* args)
{
    if(args[0] == 0) return DTVM_PTR0;
    ByteToWordEndianessTemp(sliceptr, SliceValue, args[1]);
    curr_ram[args[0]] = curr_ram[sliceptr]; // the first element is the size
    return DTVM_SUCCESS;
}*/

// Change *at* operations so that they operate on simple arrays with elements of size provided as an operand
DTVM_Signal op_at(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;

    SliceValue* val = (SliceValue*)(curr_ram + args[1]);
    dtvm_word len;
    size_t at;
    ByteToWordEndianess(sliceptr, val);
    len = curr_ram[sliceptr];
    if(curr_regs.idx >= len) return DTVM_ILLIDX;
    at = curr_ram[sliceptr + 1 + curr_regs.idx];
    //memcpy(curr_ram + args[0], curr_ram[at], sizeof(dtvm_word) * curr_ram[at + 1]);
    curr_ram[args[0]] = at;
    curr_ip += 2;
    return DTVM_SUCCESS;
}
DTVM_Signal op_atnochk(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;

    SliceValue* val = (SliceValue*)(curr_ram + args[1]);
    dtvm_word len;
    size_t at;
    ByteToWordEndianess(sliceptr, val);
    len = curr_ram[sliceptr];
    at = curr_ram[sliceptr + 1 + curr_regs.idx];
    //memcpy(curr_ram + args[0], curr_ram[at], sizeof(dtvm_word) * curr_ram[at + 1]);
    curr_ram[args[0]] = at;
    curr_ip += 2;
    return DTVM_SUCCESS;
}
DTVM_Signal op_setat(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;

    SliceValue* val = (SliceValue*)(curr_ram + args[1]);
    dtvm_word len;
    size_t at;
    ByteToWordEndianess(sliceptr, val);
    len = sliceptr;
    if(curr_regs.idx >= len) return DTVM_ILLIDX;
    at = curr_ram[sliceptr + 1 + curr_regs.idx];
    //memcpy(curr_ram + args[0], curr_ram[at], sizeof(dtvm_word) * curr_ram[at + 1]);
    curr_ram[at] = curr_ram[args[1]];
    curr_ip += 2;
    return DTVM_SUCCESS;
}
DTVM_Signal op_setatnochk(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;

    SliceValue* val = (SliceValue*)(curr_ram + args[1]);
    dtvm_word len;
    size_t at;
    ByteToWordEndianess(sliceptr, val);
    len = sliceptr;
    at = curr_ram[sliceptr + 1 + curr_regs.idx];
    //memcpy(curr_ram + args[0], curr_ram[at], sizeof(dtvm_word) * curr_ram[at + 1]);
    curr_ram[at] = curr_ram[args[1]];
    curr_ip += 2;
    return DTVM_SUCCESS;
}
// TODO: Can op_cpyregion be a good-enough low level replacement for this?

/* DTVM_Signal op_init(dtvm_byte* args)
{
    if(args[0] >= MAX_MEMORY || args[2] >= MAX_MEMORY) return DTVM_NOADDR;
    if(args[0] == 0 || args[2] == 0) return DTVM_PTR0;

    *(TypeValue*)&curr_ram[TYPTBL + args[1]]
    return DTVM_SUCCESS;
} */
DTVM_Signal op_offset(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    curr_ram[args[0]] = curr_ram[args[1] + args[2]];
    curr_ip += 2;
    return DTVM_SUCCESS;
}

// TODO: Test this instruction
// TODO: Adapt it to new memory layout and specialisation algorithm
/*DTVM_Signal op_spec(dtvm_byte* args)
{
    if(curr_csp >= MAX_CALL_DEPTH) return DTVM_CSOVFLOW;
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    curr_call = args[1];
    Value* dst = (Value*)(curr_ram + args[0]);
    Value* val = (Value*)(curr_ram + args[1]);
    Value* slc = (Value*)(curr_ram + args[2]);
    *dst = *val;
    dtvm_word depsptr  = dst->deps;
    dtvm_word len      = dst->tid_ndeps & VFIELD_SECND;

    //dtvm_word typeids_deps[len];

    for(dtvm_word i = depsptr; i < depsptr + len; i++)
    {
        Value* dep = (Value*)(curr_ram + i);
        *dep = *(Value*)(curr_ram + slc->slice + i);

        // If compiler could not figure equality of a type parameter and the type of a value parameter, it should just generate a typechecking instruction before specialising
        //if(dep->kind == KND_TYPE)
        //{
        //    typeids_deps[i] = dep->tid;
        //}
        //else if(dep->ndeps == 0)
        //{
        //}
        //else
        //{
        //    if(dep->kind == KND_CELL && ((dep->tid & 0x01) == 1))
        //    {
        //        for(size_t j = 0; j < i; j++)
        //        {
        //            if(typeids_deps[j] == dep->tid)
        //                dep->tid =
        //        }
        //    }
        //}
    }
    curr_call_stack[curr_csp++] = curr_ip;
    curr_ip = depsptr;
    return DTVM_SUCCESS;
}*/

/*DTVM_Signal op_typchk(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    curr_flags.eq = curr_ram[args[0] + TPID] == args[1];
    return DTVM_SUCCESS;
}
DTVM_Signal op_typchk_pedantic(dtvm_byte* args)
{
    if(args[0] == 0 || args[1] == 0) return DTVM_PTR0;
    curr_flags.eq = curr_ram[args[0] + TPID] == args[1];
    if(!curr_flags.eq) return DTVM_ILLTYP;
    return DTVM_SUCCESS;
}*/

DTVM_Instruction opcodes[NOPCODES] = {
    op_nop, op_halt, op_jmp, op_jmpshort, op_repeat, op_call, op_ret, op_retn, op_throw, op_catch,
    op_mov, op_cpyregion, op_cpy, op_ldr0, op_ldr1, op_ldr2, op_ldr3, op_ldrres, op_ldridx, op_str0, op_str1, op_str2,
    op_str3, op_strres, op_stridx, op_swp, op_swpregion, op_at, op_atnochk,
    op_setat, op_setatnochk, op_offset
};
