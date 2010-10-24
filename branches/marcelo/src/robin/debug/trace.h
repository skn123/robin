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
	 * A constructor which makes this traceSink a more specific
	 * versions of the general one.
	 * This tracesink will be "on" if it was enabled specifically enabled
	 * or the general (parent) sink will be enabled.
	 */
	TraceSink(TraceSink &general);

	/**
	 * @name Facet Control
	 */

	//@{
	void enable();
	void disable();
	/**
	 * Reports the state of the facet.
	 */
	inline bool on() const
	{
		if(m_general && m_general->on())
		{
			return true;
		}
		return m_active;
	}


	//@}
	/**
	 * @name Trace Out
	 */

	//@{

	inline void newLine() {
		if(on()) {
			std::cerr << std::endl;
			m_recentNewLine = true;
		}
	}

	class EndLine { };
	inline TraceSink& operator << (const EndLine& endl) {
		newLine();
		return *this;
	}


	template < class OUT >
	inline TraceSink& operator << (const OUT& out)
	{
		if (on()) {
			if(m_recentNewLine) {
				m_recentNewLine = false;
				for(int i=0;i<get_indentation();i++) {
					std::cerr << "   ";
				}
			}
			std::cerr << out;
		}
		return *this;
	}

	template < class OUT >
	void operator() (const char *label, const OUT& out)
	{
		if (on()) {
			*this << "// " << label;
			out.dbgout();
			newLine();
		}
	}

	/**
	 * Shift all debug prints right
	 */
	void increaseIndent();

	/**
	 * Shift all debug prints left;
	 */
	void decreaseIndent();
	//@}

private:
	bool m_active;

	/**
	 * How much indentation is needed.
	 */
	int m_indentation;

	inline int get_indentation() {
		if(m_general) {
			return m_general->get_indentation() + m_indentation;
		} else {
			return m_indentation;
		}
	}


	/**
	 * Whatever nothing was printed since the last new line
	 */
	bool m_recentNewLine;
	friend class Guard;

	/**
	 *  A sink which debugs a more general section of the programm
	 *  This sink will be on automatically if the general sink is on.
	 */
	TraceSink *m_general;
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

class IndentationGuard
{
public:
	IndentationGuard(TraceSink& sink);
	~IndentationGuard();

private:
	TraceSink& m_sink;
};

extern TraceSink trace;
extern TraceSink traceRegistration;
extern TraceSink traceClassRegistration;
extern TraceSink::EndLine endl;

} // end of namespace dbg

} // end of namespace Robin

#endif

