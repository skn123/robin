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


namespace Robin {

/**
 * Invokes an external function with a list of
 * arguments. The function is expected to return a word-sized
 * value.
 */
basic_block LowLevel::call_lowlevel(symbol function, const basic_block args[]) const
{
  external fcn = (external)function;
  return (*fcn)(args[0], args[1], args[2], args[3], args[4],
		args[5], args[6], args[7], args[8], args[9],
		args[10], args[11]);
}

/**
 * Invokes an external function with a list of 
 * arugments. It is assumed that the function does not return any
 * value (void).
 */
void LowLevel::call_lowlevel_void(symbol function, const basic_block args[]) const
{
  externalv fcn = (externalv)function;
  (*fcn)(args[0], args[1], args[2], args[3], args[4],
	 args[5], args[6], args[7], args[8], args[9],
	 args[10], args[11]);
}

} // end of namespace Robin

