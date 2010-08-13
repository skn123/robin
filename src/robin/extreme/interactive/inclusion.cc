#include "syntax.h"


void InteractiveSyntaxAnalyzer::IncludeIn(const char *filename, int times)
{
	if (include.stack_pointer < INCLUDE_STACK_SIZE) {
		std::istream *included = new std::ifstream(filename);
		if (included) {
			include.buf_stack[include.stack_pointer] = YY_CURRENT_BUFFER;
			include.times_stack[include.stack_pointer] = include.times;
			++include.stack_pointer;
			yy_switch_to_buffer(yy_create_buffer(included, 2048));
			include.times = times;
		}
		else {
			std::cerr << "// @ERROR: failed to open " << filename
					  << std::endl;
		}
	}
}

bool InteractiveSyntaxAnalyzer::IncludeOut()
{
	if (--include.times > 0) {
		yyrestart(yyin);
		return true; 
	}
	else if (include.stack_pointer > 0) {
		--include.stack_pointer;
		yy_switch_to_buffer(include.buf_stack[include.stack_pointer]);
		include.times = include.times_stack[include.stack_pointer];
		return true;
	}
	else
		return false;
}
