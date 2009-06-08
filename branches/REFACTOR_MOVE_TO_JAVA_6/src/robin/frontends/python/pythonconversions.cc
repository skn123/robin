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


/**
 * This conversion is possible iff the Python Long can fit inside the
 * integral C type bounded by min_bits and max_bits.
 *
 * @return the normal weight of this conversion if the conversion is
 *    possible, otherwise Weight::INFINITE.
 */
Conversion::Weight IntegralTruncate::weight(Insight insight) const
{
	if (insight.i_long >= m_min_bits && insight.i_long <= m_max_bits) {
		return Conversion::weight();
	}
	else {
		return Conversion::Weight::INFINITE;
	}
}

/**
 * Truncates long long to long.
 */
scripting_element IntegralTruncate::apply(scripting_element value) const
{
	PyObject *pyvalue = (PyObject *)value;
	Py_INCREF(pyvalue);
	return pyvalue;
}


} // end of namespace Python

} // end of namespace Robin
