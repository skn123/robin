// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-


namespace Robin {

namespace Python {

/**
 * @file
 *
 * Extracts a C++ pointer held in a PyObject that wraps a Robin instance.
 * Prior to conversion, the object's type is checked; if it does not match
 * the specified 'classname', <b>NULL</b> is returned with an approperiate
 * Python error string set.
 */
template <class T>
T *Facade::asObject(classdescriptor classname, PyObject *pyobj)
{
	// Assert the correct type of given object
	if (!check(classname, pyobj)) {
		PyErr_SetObject(PyExc_TypeError, 
			PyString_FromFormat("expected: instance of C++ class '%s'", 
								classname));
		return NULL;
	}

	// Perform actual conversion
	return asObject_trustme<T>(pyobj);
}

/**
 * Extracts a C++ pointer held in a PyObject <b>without checking</b> and 
 * "believing" that the object really wraps a Robin instance.
 *
 * @par note
 * If the object pass is not a Robin instance object, unexpected results
 * may occur.
 */
template <class T>
T *Facade::asObject_trustme(PyObject *pyobj)
{
	return (T*)object2pointer(pyobj);
}


} // end of namespace Robin::Python

} // end of namespace Robin
