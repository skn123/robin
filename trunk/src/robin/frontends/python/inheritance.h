// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 */

#ifndef ROBIN_FRONTENDS_PYTHON_INTERCEPTOR_H
#define ROBIN_FRONTENDS_PYTHON_INTERCEPTOR_H

#include <limits>
#include <Python.h>


namespace Robin {

namespace Python {


class ClassObject;

/**
 * Auxiliary class which serves as a base class in Python for classes
 * wishing to extend an abstract class.
 */
class Implementor : public PyTypeObject
{
public:
	void initialize(ClassObject *base);


	static PyObject *__new__(PyTypeObject *type, PyObject *args, PyObject* kw);

	struct Object {
		PyObject_HEAD;
		PyObject *ob_dict;
		PyObject *ob_weak;
	};

private:
	PyNumberMethods as_number;
	PySequenceMethods as_sequence;
	PyMappingMethods as_mapping;
	PyBufferProcs as_buffer;
	PyObject *name, *slots;

	ClassObject *tp_implements;
};


} // end of namespace Robin::Python

} // end of namespace Robin


#endif
