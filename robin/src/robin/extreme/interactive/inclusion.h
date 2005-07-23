#ifndef ROBIN_EXTREME_INTERACTIVE_SIMPLE_INCLUSION_H
#define ROBIN_EXTREME_INTERACTIVE_SIMPLE_INCLUSION_H


#include <fstream>

/** DO NOT INCLUDE THIS FILE EXCEPT FROM simple.yy!!! **/

#define INCLUDE_STACK_SIZE 16

struct IncludeStack {
	IncludeStack() : stack_pointer(0), times(1) { }

	struct yy_buffer_state *buf_stack[INCLUDE_STACK_SIZE];
	int                     times_stack[INCLUDE_STACK_SIZE];
	int stack_pointer;
	int times;
};



#endif
