#include "backtrace.h"

#include <execinfo.h>

namespace Robin {

Backtrace Backtrace::generateFromHere()
{
	std::vector<std::string> bt;
	
	void *trace[64];
	int trace_size = backtrace(trace, 64);
	char **messages = backtrace_symbols(trace, trace_size);

	for (int i = 0; i < trace_size; ++i) {
		bt.push_back(messages[i]);
	}

	free(messages);
	
	return Backtrace(bt);
}

Backtrace::Backtrace()
{ }

Backtrace::Backtrace(const std::vector<std::string> &trace)
	: std::vector<FrameEntry>(trace.size())	
{
	const std::string START("(");
	const std::string END("+0x");
	
	for (size_t i = 0; i < trace.size(); ++i) {
		FrameEntry &entry = (*this)[i];
		int istart = trace[i].find(START);
		int iend   = trace[i].find(END);
		entry.filename = trace[i].substr(0,istart);
		entry.function = trace[i].substr(istart + 1, iend - (istart + 1));
		entry.lineNumber = 0;
	}
}

}
