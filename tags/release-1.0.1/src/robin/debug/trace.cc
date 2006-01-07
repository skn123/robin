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


namespace dbg {

TraceSink trace;
TraceSink::EndLine endl;



/**
 */
TraceSink::TraceSink()
	: m_active(false)
{ }



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

/**
 * Reports the state of the facet.
 */
bool TraceSink::on() const
{
	return m_active;
}



TraceSink& TraceSink::operator << (const EndLine& endl)
{
	if (m_active)
		std::cerr << std::endl;
	return *this;
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

} // end of namespace dbg
