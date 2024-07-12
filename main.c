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
    dtvm_word image[6] =
    {
        0,
        OP_CPY, htonl((AMODE_LIT << 24) | (AMODE_LIT << 16) | ARITY_2), 5, 0
    };
    dtvm_init(&dtvm, nprocesses);

    dtvm_word idx = dtvm_load(&dtvm, image, 6);
    DTVM_Error result;
    if(result = dtvm_run(&dtvm, idx))
        printf("Error: %s\n", dtvm_get_error_msg(result));
    //dtvm_join(dtvm);
    dtvm_unload(&dtvm, idx);
    dtvm_destroy(&dtvm);

    return 0;
}