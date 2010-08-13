/*
 * compiler.h
 *
 *  Created on: Jan 13, 2010
 *      Author: Marcelo Taube
 */
#ifndef _PATTERN_COMPILE_H_
#define _PATTERN_COMPILE_H_

/* _UNUSEDVAR_ can be used in a variable which wont be used
   after initialization on purpose to avoid warnings.
   Example. If the declaration code was:
	   int a;
   Change it to:
       int a _UNUSEDVAR_;
*/
#ifdef __GNUC__
# define _UNUSEDVAR_ __attribute__((__unused__))
#else
# define _UNUSEDVAR_
#endif

#endif
