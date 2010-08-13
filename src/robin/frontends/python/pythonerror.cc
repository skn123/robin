/*
 * pythonerror.cc
 *
 *  Created on: Jan 27, 2010
 *      Author: Marcelo Taube
 */
#include <Python.h>
#include "pythonerror.h"
#include <stdexcept>
#include "pyhandle.h"

namespace Robin {

namespace Python {

std::string getPythonErrorAsString(std::string exceptionType, std::string doing)
{
	PyObject *exc_type=NULL, *exc_value=NULL, *exc_tb=NULL;

	// Ouch! Exception raised by a called python function.
	// Translate Python error into a C++ exception.

	PyErr_Fetch(&exc_type, &exc_value, &exc_tb);

	// PyErr_Fetch documentation states that exc_value might be null
	if(!exc_value) {
		exc_value = Py_None;
		Py_XINCREF(Py_None);
	}

	// PyErr_Fetch documentation states that exc_tb might be null
	if(!exc_tb) {
			exc_tb = Py_None;
			Py_XINCREF(Py_None);
	}


	//make sure that the three references are released
	PyReferenceSteal<PyObject> hexc_type(exc_type), hexc_value(exc_value), hexc_tb(exc_tb);

	// - format an error string
	std::string error_string = exceptionType +" - Robin was trying to convert from one type to another using "
			"python code defined by the user, but the code threw an exception.\n"
			"The output of the causing exception:\n\t"
			;

	PyReferenceSteal<PyObject> mod(PyImport_ImportModule("traceback"));
	if(!mod) {
		throw std::runtime_error(std::string("Robin fatal error:: could not load module traceback, "
				" while trying to report an exception that happened in python code,"
				" when ") + doing);
	}

	PyReferenceSteal<PyObject> list(PyObject_CallMethod(mod.pointer(), "format_exception", "OOO", exc_type, exc_value, exc_tb));
	if(!list) {
		throw std::runtime_error(std::string("Robin fatal error:: could not call traceback.format_exception, "
						" while trying to report an exception that happened in python code,"
						" when ") + doing);
	}

	PyReferenceSteal<PyObject> string(PyString_FromString("\t"));
	if(!string) {
		throw std::runtime_error( std::string("Robin fatal error:: could not create a python string object, "
						" while trying to report an exception that happened in python code,"
						" when ") + doing);
	}

	PyReferenceSteal<PyObject> join(PyUnicode_Join(string.pointer(), list.pointer()));
	if(!join) {
		throw std::runtime_error( std::string("Robin fatal error:: could not join a list of string objects, "
						" while trying to report an exception that happened in python code,"
						" when ") + doing);
	}

	error_string += PyString_AsString(join.pointer());

	PyErr_Clear();
	// return error
	return error_string;
}

void reportPythonError(std::string exceptionType, std::string doing)
{
	throw std::runtime_error(getPythonErrorAsString(exceptionType, doing));
}

} // end of namespace Robin::Python
} // end of namespace Robin
