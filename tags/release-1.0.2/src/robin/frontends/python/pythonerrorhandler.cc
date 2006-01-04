#include "pythonerrorhandler.h"

#include <Python.h>
#include <frameobject.h>

#include <string>

#include <iostream>
using namespace std;

namespace Robin {

namespace Python {

namespace {
		
	static PyFrameObject* createStackFrame(
			const std::string &funcname,
			const std::string &filename = "",
			int line_number = 0)
	{
		PyObject *py_srcfile    = NULL;
		PyObject *py_funcname   = NULL;
		PyObject *py_globals    = NULL;
		PyObject *empty_tuple   = NULL;
		PyObject *empty_string  = NULL;
		PyCodeObject *py_code   = NULL;
		PyFrameObject *py_frame = NULL;
    
		py_srcfile = PyString_FromString(filename.c_str());
		if (!py_srcfile) return NULL;
		py_funcname = PyString_FromString(funcname.c_str());
		if (!py_funcname) return NULL;
		py_globals = PyDict_New();
		if (!py_globals) return NULL;
		empty_tuple = PyTuple_New(0);
		if (!empty_tuple) return NULL;
		empty_string = PyString_FromString("");
		if (!empty_string) return NULL;

		// create the code object
		py_code = PyCode_New(
			0,            // int argcount,
			0,            // int nlocals,
			0,            // int stacksize,
			0,            // int flags,
			empty_string, // PyObject *code,
			empty_tuple,  // PyObject *consts,
			empty_tuple,  // PyObject *names,
			empty_tuple,  // PyObject *varnames,
			empty_tuple,  // PyObject *freevars,
			empty_tuple,  // PyObject *cellvars,
			py_srcfile,   // PyObject *filename,
			py_funcname,  // PyObject *name,
			line_number,  // int firstlineno,
			empty_string  // PyObject *lnotab
		);
		if (!py_code) return NULL;

		// create the stack frame object
		py_frame = PyFrame_New(
			PyThreadState_Get(), // PyThreadState *tstate,
			py_code,             // PyCodeObject *code,
			py_globals,          // PyObject *globals,
			NULL                 // PyObject *locals
		);
		if (!py_frame) return NULL;
		py_frame->f_lineno = line_number;

		// cleanup
		Py_XDECREF(py_srcfile);
		Py_XDECREF(py_funcname);
		Py_XDECREF(py_globals);
		Py_XDECREF(empty_tuple);
		Py_XDECREF(empty_string);
		Py_XDECREF(py_code);

		return py_frame;
	}

}


PythonErrorHandler::PythonErrorHandler()
	: m_error(NULL)
{ }

void PythonErrorHandler::setError(scripting_element error)
{
	m_error = error;
}

void PythonErrorHandler::setError(const std::exception &exc,
	                              const Backtrace &trace)
{
	// add all functions to traceback
	for (int i = 0; i < trace.size(); ++i) {
		if (trace[i].function.find("Robin") < trace[i].function.length() ||
		    trace[i].function.find("Py")    < trace[i].function.length())
			continue;

		PyFrameObject *frame = createStackFrame(
				trace[i].function, trace[i].filename, trace[i].lineNumber);
		PyTraceBack_Here(frame);
	}

	// retreive traceback
	PyThreadState *state = PyThreadState_Get();
	PyTracebackObject *t = (PyTracebackObject*) state->curexc_traceback;
	Py_XINCREF((PyObject*)t);
	
	// create runtime error
	PyObject *excargs = Py_BuildValue("(s)", exc.what());
	PyObject *pyexc   = PyObject_Call(
			PyExc_RuntimeError, excargs, NULL);

	// set error info
	PyObject *errinfo = Py_BuildValue(
			"OOO", PyExc_RuntimeError, pyexc, (PyObject*)t);
	setError(errinfo);

	// cleanup
	Py_XDECREF(excargs);
	Py_XDECREF(pyexc);
}

scripting_element PythonErrorHandler::getError()
{
	return m_error;
}

}

}
