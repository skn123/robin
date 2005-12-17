#include "backtrace.h"

/*** Linux Backtrace ***/
#if defined(__linux) && defined(WITH_LIBERTY)
#include <execinfo.h>
#include "../debug/demangle.h"
namespace {

	std::vector<std::string> acquireBacktrace()
	{
		std::vector<std::string> bt;
		
		void *trace[64];
		int trace_size = backtrace(trace, 64);
		char **messages = backtrace_symbols(trace, trace_size);
		
		for (int i = 0; i < trace_size; ++i) {
			bt.push_back(messages[i]);
		}

		free(messages);

		return bt;
	}

	std::string demangle(const std::string &mangled)
	{
		char *demangled = cplus_demangle(mangled.c_str(), AUTO_DEMANGLING);
		if (demangled) return demangled;
		return mangled;
	}
	
}
/*** Stubs for Unsupported Platforms ***/
#else
namespace {

	std::vector<std::string> acquireBacktrace()
	{
		return std::vector<std::string>();
	}

	std::string demangle(const std::string &mangled)
	{
		return mangled;
	}
	
}
#endif

namespace Robin {

Backtrace Backtrace::generateFromHere()
{
	return Backtrace(acquireBacktrace());
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

		entry.function = demangle(entry.function);
	}
}

}
