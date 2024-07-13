#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>

#include "common.h"
#include "vm.h"
// ifc? indexed function count? independent function counter? if counter?

int main(int argc, char** argv)
{
    DTVM dtvm;
    dtvm_word nprocesses = 1;
    dtvm_word image[80] =
    {
        0,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 1, 2,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 2, htonl((AMODE_LIT << 24) | ARITY_1),
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 3, 16,
        OP_JMP, htonl((AMODE_LIT << 24) | ARITY_1), 1,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 18, 18,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 22, 22,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 26, 26,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 30, 30,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 34, 34,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 38, 38,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 42, 42,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 46, 46,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 50, 50,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 54, 54,
        OP_MOV, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 58, 58,
        OP_NOP, htonl(ARITY_0),
        OP_NOP, htonl(ARITY_0),
        OP_NOP, htonl(ARITY_0),
        OP_NOP, htonl(ARITY_0),
        OP_NOP, htonl(ARITY_0),
        OP_NOP, htonl(ARITY_0),
        OP_NOP, htonl(ARITY_0),
        OP_NOP, htonl(ARITY_0),
        OP_NOP, htonl(ARITY_0),
        OP_HALT, htonl(ARITY_0)
    };
    dtvm_init(&dtvm, nprocesses);

    dtvm_word idx = dtvm_load(&dtvm, image, 80);
    DTVM_Error result;
    if(result = dtvm_run(&dtvm, idx))
    {
        printf("Error: %s\n", dtvm_get_error_msg(result));
        printf("Near IP: %ld\n", curr_ip);
        printf("Memcell: %ld\n", curr_ram[curr_ip]);
    }
    //dtvm_join(dtvm);
    dtvm_unload(&dtvm, idx);
    dtvm_destroy(&dtvm);

    return 0;
}