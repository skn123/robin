// -*- C++ -*-

/**
 * @file
 *
 * @par SOURCE
 * python_low_level.cc
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

// System includes
#include <Python.h>
#include <pythread.h>

#include <map>

// Package includes
#include "pythonlowlevel.h"

#ifdef _WIN32
extern __declspec(dllimport) PyThreadState *_PyThreadState_Current;
#else
extern PyThreadState *_PyThreadState_Current;
#endif


namespace Robin {

namespace Python {


namespace {
	std::map<int, PyThreadState *> saved_threads;
}



/**
 * Invokes an external function with a list of
 * arguments. The function is expected to return a word-sized
 * value.
 * This method wraps the original with a thread state save,
 * so that this call can be multi-threaded in python.
 */
basic_block PythonLowLevel::call_lowlevel(symbol function, const basic_block* args) const
{
	ThreadStateGuardian guard;
	return LowLevel::call_lowlevel(function, args);
}

/**
 * Invokes an external function with a list of 
 * arugments. It is assumed that the function does not return any
 * value (void).
 * This method wraps the original with a thread state save,
 * so that this call can be multi-threaded in python.
 */
void PythonLowLevel::call_lowlevel_void(symbol function, const basic_block* args) const
{
	ThreadStateGuardian guard;
	LowLevel::call_lowlevel_void(function, args);
}


/**
 * Stores Python's current thread state.
 */
ThreadStateGuardian::ThreadStateGuardian()
{
	_save = PyEval_SaveThread();
	saved_threads[PyThread_get_thread_ident()] = _save;
}

/**
 * Restores Python's thread state in order to allow the Python environment
 * to carry on normally.
 */
ThreadStateGuardian::~ThreadStateGuardian()
{
	PyEval_RestoreThread(_save);

	// TODO: threads are never deleted from the map, may induce a memory leak
}

/**
 * Locates the Python thread state associated with the current thread and
 * temporarily re-activates it to allow execution of Python code within a
 * running C thread.
 */
AntiThreadStateGuardian::AntiThreadStateGuardian()
	: m_restored(false)
{
	// Acquire thread state
	PyThreadState *current = _PyThreadState_Current;
	PyThreadState *&pending = saved_threads[PyThread_get_thread_ident()];

	if (current == NULL || (pending != NULL && current != pending)) {
		// - possibly create a new PyThreadState if none exist
		if (!pending) {
			PyInterpreterState *interp = PyInterpreterState_Head();
			pending = PyThreadState_New(interp);
		}
		// Acquire the current Python thread
		PyEval_RestoreThread(pending);
		m_restored = true;
	}
}

/**
 * Resumes a C thread by storing the current Python thread aside.
 */
AntiThreadStateGuardian::~AntiThreadStateGuardian()
{
	if (m_restored) {
		PyThreadState *tstate = PyEval_SaveThread();
	}

	// TODO: PyThreadStates created by the constructor are never disposed of.
}


} // end of namespace Robin::Python

} // end of namespace Robin

