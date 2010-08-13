/*
 * pylegalconversions.h
 *
 *  Created on: Jan 26, 2010
 *      Author: Marcelo Taube
 *
 *  Python objects are implemented in C thus they do not have the
 *  option of inheriting from the struct PyObject, automatic conversions
 *  are not defined from them to its superclass.
 *
 *  This template function can be used by generic code to make sure
 *  that a casting is valid (instead of using directly a cast).
 *  The code uses specialization, a new version of the function
 *  convertToPyObject should be added gradually for each type that
 *  can be convertible to PyObject but do not have an automatic operation.
 *
 *
 */

#ifndef ROBIN_FRONTENDS_PYTHON_PYLEGALCONVERSIONS_H
#define ROBIN_FRONTENDS_PYTHON_PYLEGALCONVERSIONS_H

/**
 * It is a function which just returns the same pointer which it recieves
 * and performing an implicit cast.
 * The difference between an implicit cast is that it works only for
 * valid types, not unrelated types. We want to be sure that the parameter
 * can be casted to type PyObject, which means its a python object.
 *
 */
template<typename T>
inline PyObject *convertToPyObject(T*obj) {
		// The next line will issue an error if T is not
		// convertible to PyObject
		//
		// Since the python implementation does not define conversions
		// between its types and is written in plain C (no inheritance)
		// the compiler will fail for all python types not defined in
		// robin. The solution is to add explicit specializations for
		// each type T which we are sure is a legal python object.
		// But do not add indiscriminately, examine the code and see why
		// convertToPyObject was called at all and whatever the code running
		// is logical.
		return obj;

}

template<>
inline PyObject *convertToPyObject<PyIntObject>(PyIntObject *obj) {
		return (PyObject*)obj;
}

template<>
inline PyObject *convertToPyObject<PyLongObject>(PyLongObject *obj) {
		return (PyObject*)obj;
}
#endif
