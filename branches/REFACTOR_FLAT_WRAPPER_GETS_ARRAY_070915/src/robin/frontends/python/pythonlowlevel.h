// -*- C++ -*-

/**
 * @file
 *
 * @par SOURCE
 * python_low_level.h
 *
 * @par TITLE
 * Python wrapper for Low Level call in C.
 *
 * @par PUBLIC-FUNCTIONS
 *
 * <ul><li>call_lowlevel</li>
 *     <li>call_lowlevel_void</li>
 * </ul>
 */

#ifndef ROBIN_PYTHON_LOWLEVEL_H
#define ROBIN_PYTHON_LOWLEVEL_H


// Package includes
#include <robin/reflection/low_level.h>

namespace Robin {

namespace Python {


/**
 * @class PythonLowLevel
 * @nosubgroups
 *
 * <p>This is the low level interface to the actual functions to
 * be called.</p>
 * <p>Using the low level calls, functions are given actual memory
 * blocks, and not their ArgumentBuffer representation, and then
 * call the relevant function, finally returning the actual return
 * type (adapted back to the original type or just as a machine word).</p>
 * <p>This implementation wraps the calls with a thread lock, so that
 * they can be invoked in a multi-threaded script.</p>
 */
class PythonLowLevel : public LowLevel {

public:

	/**
	 * @name Call
	 */
	//@{
	virtual
	basic_block call_lowlevel(symbol function, const basic_block* args) const;
	
	virtual
	void call_lowlevel_void(symbol function, const basic_block* args) const;

	//@}

	/**
	 * @name Destructor
	 */
	//@{
	virtual ~PythonLowLevel() {}

	//@}

};


/**
 * Stores and restores the Python interpreter's thread-state upon entry
 * and exit of a function call.
 */
class ThreadStateGuardian
{
public:
	ThreadStateGuardian();
	~ThreadStateGuardian();

private:
	PyThreadState *_save;
};

/**
 * Stores and restores the Python interpreter's thread-state upon entry
 * and exit of a function call.
 */
class AntiThreadStateGuardian
{
public:
	AntiThreadStateGuardian();
	~AntiThreadStateGuardian();

private:
	bool m_restored;
};


} // end of namespace Robin::Python

} // end of namespace Robin

#endif
