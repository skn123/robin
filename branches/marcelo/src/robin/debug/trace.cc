// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * trace.cc
 *
 * Provides debugging utilities for embedding trace debug
 * printouts in internal package routines. A global trace flag is introduced
 * to control whether or not traces are actually sent to the console.
 * Trace points are printed to std::cerr in a form of comment blocks (such
 * as this one), so concise HTML documents can be easily generated from
 * produced output.
 */

#include "trace.h"


namespace Robin {
namespace dbg {

TraceSink trace;
TraceSink traceRegistration(trace);
TraceSink traceClassRegistration(traceRegistration);
TraceSink::EndLine endl;



/**
 */
TraceSink::TraceSink()
	: m_active(false), m_indentation(0), m_recentNewLine(false), m_general(0)
{ }

TraceSink::TraceSink(TraceSink &general)
	: m_active(false), m_indentation(0), m_recentNewLine(false), m_general(&general)
{


}
/**
 * Turns on trace output to this sink.
 */
void TraceSink::enable()
{
	m_active = true;
}

/**
 * Turns off trace output sent to this sink.
 */
void TraceSink::disable()
{
	m_active = false;
}



void TraceSink::increaseIndent() {
	m_indentation++;
}
void TraceSink::decreaseIndent() {
	m_indentation--;
}


/**
 */
Guard::Guard(TraceSink& sink)
	: m_sink(sink)
{
	m_restore = m_sink.m_active;
	m_sink.disable();
}

/**
 * Restores the original state of the sink.
 */
Guard::~Guard()
{
	if (m_restore) m_sink.enable();
}


IndentationGuard::IndentationGuard(TraceSink &sink)
	: m_sink(sink)
{
	m_sink.increaseIndent();
}

IndentationGuard::~IndentationGuard()
{
	m_sink.decreaseIndent();
}

} // end of namespace dbg
} // end of namespace Robin
