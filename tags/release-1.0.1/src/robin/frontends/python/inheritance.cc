// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#include "inheritance.h"

#include "pythonobjects.h"


namespace Robin {

namespace Python { 


PyTypeObject NullTypeObject = {
	PyObject_HEAD_INIT(&PyType_Type)
    0,
    "Robin::Python::NullObject",
    sizeof(PyObject),
    0,
    0, /*tp_dealloc*/
    0,                           /*tp_print*/
    0, /*tp_getattr*/
    0,                           /*tp_setattr*/
    0,                           /*tp_compare*/
	0,    /*tp_repr*/
    0,                           /*tp_as_number*/
    0,                           /*tp_as_sequence*/
    0,                           /*tp_as_mapping*/
    0,                           /*tp_hash */
	0     /*tp_call*/
};



#ifndef offsetof
#define offsetof(type, member) ( (int) & ((type*)0) -> member )
#endif

/**
 * Prepares the implementor type by filling appropriate fields of 
 * PyTypeObject.
 */
void Implementor::initialize(ClassObject *base)
{
	(PyTypeObject&)(*this) = NullTypeObject;

	ob_size = sizeof(Implementor);

    tp_name = (char *)malloc(strlen(base->tp_name) + 1);
	strcpy(tp_name, base->tp_name);
	tp_basicsize = sizeof(Object);

	tp_dictoffset = offsetof(Object, ob_dict);
	tp_weaklistoffset = offsetof(Object, ob_weak);

	// Support the new class model
	tp_flags |= Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_CLASS | 
		        Py_TPFLAGS_CHECKTYPES;
	tp_base = &PyBaseObject_Type;

	tp_mro = NULL;
	tp_new = __new__;

	name = slots = NULL;

	tp_implements = base;
	Py_XINCREF(tp_implements);
}


/**
 * Creates a new instance of the implementor object. This is not intended to
 * be invoked directly but rather through creation of a derived class from
 * the original implementor. It allocates a new object using the type's own
 * tp_alloc and sets a member to a new instance of the interceptor object for
 * that interface.
 */
PyObject *Implementor::__new__(PyTypeObject *type,
							   PyObject *args, PyObject *kw)
{
	PyTypeObject *btype;

	if (type->tp_alloc == 0) {
		PyErr_Format(PyExc_TypeError, "cannot create '%s' instances",
					 type->tp_name);
		return NULL;
	}

	// Find the implementor
	// TODO verify that this comparison always works
	for (btype = type; btype->tp_base != &PyBaseObject_Type; btype = btype->tp_base);
	Implementor *implementor = (Implementor*)btype;
	ClassObject *interface = implementor->tp_implements;

	// Instantiate an interceptor object
	PyObject *implementation = interface->tp_new(interface, args, kw);
	if (implementation == NULL)
		return NULL;

	// Allocate a new object with a reference to the implementation just
	// created
	Object *obj = (Object*)type->tp_alloc(type, 0);
	obj->ob_dict = PyDict_New();
	obj->ob_weak = NULL; //?? PyList_New(0);

	PyDict_SetItemString(obj->ob_dict, interface->tp_name, implementation);

	// Initialize the interceptor
	PyObject_CallMethod(implementation, "_init", "O", obj);

	return (PyObject*)obj;
}



} // end of namespace Robin::Python

} // end of namespace Robin
