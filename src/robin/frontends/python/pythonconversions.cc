// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/simple/pythonconversions.h
 *
 * @par TITLE
 * Sub-classes of Conversion for the Python Interpreter
 *
 * @par PACKAGE
 * Robin
 *
 * Basic conversions which are supplied by the Python Interpreter
 * for built-in conversions between types.
 *
 * @par PUBLIC-CLASSES
 * <ul><li>TrivialConversion</li><li>StringToPascalStringConversion</li></ul>
 */

#include <Python.h>

#include "pythonconversions.h"
#include "pythonobjects.h"


namespace Robin {

namespace Python {

/**
 * Promotes an integer value to a float value (precision loss may occur).
 */
scripting_element IntToFloatConversion::apply(scripting_element value) const
{
	PyObject *pyint = (PyObject*)value;
	return PyFloat_FromDouble(PyInt_AsLong(pyint));
}



} // end of namespace Python

} // end of namespace Robin
