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
basic_block LowLevel::call_lowlevel(symbol function, const basic_block* args) const {
    // NOTE: Caution! This function is extremly fregile! Handle with care!
    external fcn = (external)function;
    return fcn(args);
}

/**
 * Invokes an external function with a list of 
 * arugments. It is assumed that the function does not return any
 * value (void).
 */
void LowLevel::call_lowlevel_void(symbol function, const basic_block* args) const {
    // NOTE: Caution! This function is extremly fregile! Handle with care!
    externalv fcn = (externalv)function;
    fcn(args);
}
} // end of namespace Robin

