#ifndef ROBIN_REFLECTION_BACKTRACE
#define ROBIN_REFLECTION_BACKTRACE

#include <vector>
#include <string>

namespace Robin {

/**
 * This is a single stack frame entry in the backtrace object.
 */
struct FrameEntry
{
	std::string function;
	std::string filename;
	int         lineNumber;
};

/**
 * This class can hold a stack trace of a function, and allow easy access and
 * manipulation of that stack trace.
 */
class Backtrace : public std::vector<FrameEntry>
{
public:
	/**
	 * Generates a backtrace object from this position to the top.
	 *
	 * @return a backtrace object starting at this position
	 */
	static Backtrace generateFromHere();
	
public:
	/**
	 * Default constructor creates an empty backtrace.
	 */
	Backtrace();
	
	/**
	 * Constructs the backtrace object from a list of strings, each written with
	 * the format depicted in the system backtrace functions.
	 *
	 * @param trace a vector of strings conforming to the backtrace standard
	 */
	Backtrace(const std::vector<std::string> &trace);
};

}

#endif
