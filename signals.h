#ifndef DTVM_SIGNALS_H
#define DTVM_SIGNALS_H

typedef enum
{
    DTVM_SUCCESS,
    DTVM_PTR0,
    DTVM_CSUDFLOW,
    DTVM_CSOVFLOW,
    DTVM_NOADDR,
    DTVM_NOREG,
    DTVM_ILLKND,
    DTVM_ILLIDX,
    DTVM_NOSPACE
} DTVM_Signal;

const char* dtvm_get_signal_msg(DTVM_Signal sig);

#endif
