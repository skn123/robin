// -*- C++ -*-

/**
 * @file
 *
 * @par SOURCE
 * low_level.cc
 *
 * @par TITLE
 * Low Level call in C.
 *
 * @par PUBLIC-FUNCTIONS
 *
 * <ul><li>call_lowlevel</li>
 *     <li>call_lowlevel_void</li>
 * </ul>
 */

#include "low_level.h"
#include <cstring>
#include <alloca.h>

namespace Robin {

/**
 * Invokes an external function with a list of
 * arguments. The function is expected to return a word-sized
 * value.
 */
basic_block LowLevel::call_lowlevel(symbol function, const basic_block* args, size_t argsCount) const {
    // NOTE: Caution! This function is extremly fregile! Handle with care!
    external fcn = (external)function;
#ifdef IS_ARGUMENT_LIMIT
    return (*fcn)(args[0], args[1], args[2], args[3], args[4],
     	args[5], args[6], args[7], args[8], args[9],
     	args[10], args[11]);
#else
    register size_t i = 0;
    basic_block* sp = (basic_block*)alloca(argsCount * sizeof(basic_block));
    // NOTE: Can't use here memcpy (or any alternative), because they allocate
    // variables on the stack!
    for (i = 0; i < argsCount; ++i) {
        sp[i] = args[i];
    }
    return (*fcn)();
#endif
}

/**
 * Invokes an external function with a list of 
 * arugments. It is assumed that the function does not return any
 * value (void).
 */
void LowLevel::call_lowlevel_void(symbol function, const basic_block* args, size_t argsCount) const {
    // NOTE: Caution! This function is extremly fregile! Handle with care!
    externalv fcn = (externalv)function;
#ifdef IS_ARGUMENT_LIMIT
    (*fcn)(args[0], args[1], args[2], args[3], args[4],
     	args[5], args[6], args[7], args[8], args[9],
     	args[10], args[11]);
#else
    register size_t i = 0;
    basic_block* sp = (basic_block*)alloca(argsCount * sizeof(basic_block));
    // NOTE: Can't use here memcpy (or any alternative), because they allocate
    // variables on the stack!
    for (i = 0; i < argsCount; ++i) {
        sp[i] = args[i];
    }
    (*fcn)();
#endif
}
} // end of namespace Robin

