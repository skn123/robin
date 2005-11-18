// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#include "pythoninterceptor.h"

#include <Python.h>
#include <stdexcept>
#include <assert.h>

#include "../../debug/trace.h"
#include "pythonlowlevel.h"
#include "pythonerrorhandler.h"


namespace Robin {

namespace Python {



PythonInterceptor::~PythonInterceptor()
{
}

/**
 * Triggers a Python object method in response to the callback.
 *
 * @param twin a Python object containing the implementation of the callback
 * @param signature specification for the method being invoke
 * @param args arguments as received by function wrapper in C++
 * @returns basic_block The result of the function call.
 */
basic_block PythonInterceptor::callback(scripting_element twin,
								 const Signature& signature,
								 basic_block args[]) const
{
	AntiThreadStateGuardian guard;

	PyObject *pytwin = (PyObject*)twin;

	// Construct a tuple of arguments
	int nargs = signature.argumentTypes.size();
	PyObject *pyargs = PyTuple_New(nargs);

	for (int argi = 0; argi < nargs; ++argi) {
		Handle<TypeOfArgument> argumentType = signature.argumentTypes[argi];

		PyObject *pyarg = (PyObject*)argumentType->get(args[argi]);
		PyTuple_SET_ITEM(pyargs, argi, pyarg);
	}

	if (dbg::trace.on()) {
		PyObject_Print(pytwin, stderr, 0);
		PyObject_Print(pyargs, stderr, 0);
		fprintf(stderr, "\n");
	}

	// Find python method
	char *methname = const_cast<char*>(signature.name.c_str());
	PyObject *method = PyObject_GetAttrString(pytwin, methname);
	if (!method) {
		throw std::runtime_error("method '" + signature.name + 
								 "' is not implemented.");
	}
	// Invoke python method
	PyObject *result = PyObject_CallObject(method, pyargs);
	Py_XDECREF(method);
	Py_XDECREF(pyargs);
	if (!result) {
		reportCallbackError();
	}

	if (dbg::trace.on()) {
		fprintf(stderr, "// PythonInterceptor::callback: returned '");
		PyObject_Print(result, stderr, 0); fprintf(stderr, "'\n");
	}

	basic_block returnValue = 0;
	
	// Translate returned value
	if (signature.returnType) {
		// - detect the result type
		Handle<TypeOfArgument> detectedType =
			FrontendsFramework::activeFrontend()->detectType(result);
		if (detectedType != signature.returnType) {
			throw std::runtime_error("method '" + signature.name +
			                         "' does not return the expected type.");
		}
		// - acquite the adapter for the function's return type
		Handle<Adapter> adapter = FrontendsFramework::activeFrontend()
			->giveAdapterFor(*(signature.returnType));
		// - create an arguments buffer with the result put on
		ArgumentsBuffer argbuff;
		adapter->put(argbuff, result);
		// - return the basic block off the buffer
		returnValue = argbuff.getBuffer()[0];
	}
	
	// Cleanup
	Py_XDECREF(result);

	return returnValue;
}


/**
 * Throws std::runtime_error with a description of the Python error that
 * just occurred.
 */
void PythonInterceptor::reportCallbackError() const
{
	// Ouch! Exception raised by callback function.
	// Translate Python error into a C++ exception.
	// However, preserve the original stack trace for later
	PyObject *exc_type, *exc_value, *exc_tb;
	PyErr_Fetch(&exc_type, &exc_value, &exc_tb);
	// - store the error information
	PyObject *errinfo = Py_BuildValue("OOO", exc_type, exc_value, exc_tb);
	FrontendsFramework::activeFrontend()->getErrorHandler().setError(errinfo);
	// - process the error information
	PyObject *exc_type_repr = PyObject_GetAttrString(exc_type, "__name__");
	PyObject *exc_value_repr = PyObject_Str(exc_value);
	// - format an error string
	std::string error_string = "Python error occurred inside callback - ";
	error_string += PyString_AsString(exc_type_repr);
	(error_string += ": ") += PyString_AsString(exc_value_repr);
	// - cleanup
	Py_XDECREF(exc_type); Py_XDECREF(exc_value); Py_XDECREF(exc_tb);
	Py_XDECREF(exc_type_repr); Py_XDECREF(exc_value_repr);
	PyErr_Clear();
	// - throw
	throw std::runtime_error(error_string);
}


} // end of namespace Robin::Python

} // end of namespace Robin

