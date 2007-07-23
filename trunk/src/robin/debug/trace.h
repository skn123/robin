// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * trace.h
 *
 * Provides debugging utilities for embedding trace debug
 * printouts in internal package routines. A global trace flag is introduced
 * to control whether or not traces are actually sent to the console.
 * Trace points are printed to std::cerr in a form of comment blocks (such
 * as this one), so concise HTML documents can be easily generated from
 * produced output.
 */

#ifndef ROBIN_DEBUG_TRACE_H
#define ROBIN_DEBUG_TRACE_H

#include <iostream>

namespace Robin {

/**
 * @brief Utilities for debug printouts and traces.
 */
namespace dbg {

/**
 * @class TraceSink
 * @nosubgrouping
 *
 * An object which accepts trace prints and prints them
 * depending upon whether or not the flag is set.
 */
class TraceSink
{
public:
	TraceSink();

	/**
	 * @name Facet Control
	 */

	//@{
	void enable();
	void disable();
	bool on() const;

	//@}
	/**
	 * @name Trace Out
	 */

	//@{

	class EndLine { };

	TraceSink& operator << (const EndLine& endl);

	template < class OUT >
	TraceSink& operator << (const OUT& out)
		{ if (m_active) std::cerr << out; return *this; }
	template < class OUT >
	void operator() (const char *label, const OUT& out) 
	{  if (m_active) {
		std::cerr << "// " << label;
		out.dbgout();
		std::cerr << std::endl;
	} }

	//@}

private:
	bool m_active;

	friend class Guard;
};

/**
 * @class Guard
 * @nosubgrouping
 *
 * Allows a calling function to temporarily disable
 * debug printing in a certain section.
 */
class Guard
{
public:
	Guard(TraceSink& sink);
	~Guard();

private:
	TraceSink& m_sink;
	bool m_restore;
};

extern TraceSink trace;
extern TraceSink::EndLine endl;

} // end of namespace dbg

} // end of namespace Robin

#endif

