#include "signals.h"

const char* dtvm_get_signal_msg(DTVM_Signal sig)
{
    switch(sig)
    {
    case DTVM_SUCCESS: return "Success";
    case DTVM_PTR0:    return "Attempted to access value at address 0";
    case DTVM_CSUDFLOW:return "Call stack underflow";
    case DTVM_CSOVFLOW:return "Call stack overflow";
    case DTVM_NOADDR:  return "Attempted to access value at a non-existent address";
    case DTVM_NOREG:   return "Attempted to access non-existent register";
    case DTVM_ILLKND:  return "Wrong kind";
    case DTVM_ILLTYP:  return "Wrong type";
    case DTVM_ILLIDX:  return "Specified index is out of bounds";
    case DTVM_NOSPACE: return "Not enough space";
    }
}

