// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#include "inheritance.h"

#include "pythonobjects.h"
#include "../../reflection/class.h"
#include "../../reflection/instance.h"


namespace Robin {

namespace Python { 


PyTypeObject NullTypeObject_ = {
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

#define offsetof_nw(type, member) ((char*)&((type*)1)->member - (char*)1)


PyTypeObject *HybridTypeObject = 0;

HybridObject::HybridObject(PyTypeObject *obtype)
{
	PyObject_Init(this, obtype);
	m_dict = PyDict_New();
}

HybridObject::~HybridObject()
{
	Py_XDECREF(m_dict);
}

/**
 * Creates and initializes a new Hybrid object.
 */
PyObject *HybridObject::__new__(PyTypeObject *classtype,
						 PyObject *args, PyObject *kw) 
{
    HybridObject *hybrid = new HybridObject(classtype);

	PyObject *value;
	PyObject *rc;

	// - try to invoke derived class __init__, fall back to C++ constructor
	if (value = PyDict_GetItemString(classtype->tp_dict, "__init__")) {
		rc = PyObject_Call(PyMethod_New(value, hybrid, PyObject_Type(hybrid)), 
						   args, NULL);
		if (rc && !hybrid->isInitialized()) {
			PyObject *noargs = PyTuple_New(0);
			rc = ClassObject::__init__(hybrid, noargs);
			Py_XDECREF(noargs);
		}
	}
	else {
		PyErr_Clear();
		rc = ClassObject::__init__(hybrid, args);
	}

	// - try to invoke _init (interceptors' facility)
	if (rc) {
		if (value = hybrid->getBoundMethodOrDataMember("_init")) {
			PyObject *twin = PyTuple_New(1);
			PyTuple_SetItem(twin, 0, hybrid);
			rc = PyObject_Call(value, twin, NULL);
			Py_XDECREF(twin);
		}
		else {
			PyErr_Clear();
		}
	}

	// - check for errors
	if (rc == NULL) {
		delete hybrid;
		return NULL;
	}

	return hybrid;
}

/**
 * Destroys and deallocates a Hybrid object.
 */
void HybridObject::__del__(PyObject *object)
{
	delete (HybridObject*)object;
}

PyObject *HybridObject::__getattr__(PyObject *self, char *nm)
{
	return ((HybridObject*)self)->__getattr__(nm);
}

int HybridObject::__setattr__(PyObject *self, char *nm, PyObject *value)
{
	return ((HybridObject*)self)->__setattr__(nm, value);
}

/**
 * Hybrid object attribute access:
 */
PyObject *HybridObject::__getattr__(char *nm)
{
	PyObject *value;

	if (value = PyDict_GetItemString(m_dict, nm)) {
		Py_XINCREF(value);
		return value;
	}
	else {
		PyErr_Clear();
		if (value = PyDict_GetItemString(ob_type->tp_dict, nm)) {
			return PyMethod_New(value, this, PyObject_Type(this));
		}
		else {
			PyErr_Clear();
			return InstanceObject::__getattr__(nm);
		}
	}
}

int HybridObject::__setattr__(char *nm, PyObject *value)
{
	Py_XINCREF(value);
	return PyDict_SetItemString(m_dict, nm, value);
}

/**
 * Makes a new Hybrid class object. This is not intended to be invoked
 * directly but rather through creation of a derived class from an
 * existing Robin::Python::ClassObject.
 */
PyObject *HybridObject::__new_hybrid__(PyTypeObject *metaclasstype,
									   PyObject *args, PyObject *kw) 
{
	char *name;
	static char *kwlist[] = {"name", "bases", "dict", 0};
	PyObject *bases, *dict;

	PyArg_ParseTupleAndKeywords(args, kw, "sOO", kwlist,
								&name, &bases, &dict);

	int nbases = PyTuple_Size(bases);
	Handle<Class> underlyingBase;
	ClassObject *baseClass;
	for (int i = 0; i < nbases; ++i) {
		PyObject *base = PyTuple_GET_ITEM(bases, i);
		if (ClassObject_Check(base)) {
			if (underlyingBase) {
				PyErr_SetString(PyExc_TypeError, "Robin is unable to support "
								"multiple inheritance at this time");
				return NULL;
			}
			baseClass = (ClassObject*)base;
			underlyingBase = baseClass->getUnderlying();
		}
	}

	assert(underlyingBase);

    PyTypeObject *newtype = new ClassObject(*baseClass);
    if (newtype == NULL) return NULL;
	newtype->ob_type = HybridTypeObject;
	newtype->tp_name = strdup(name);
    newtype->tp_new = __new__;
    newtype->tp_dealloc = __del__;
	
    newtype->tp_getattr = HybridObject::__getattr__;
    newtype->tp_getattro = 0;
	newtype->tp_setattr = HybridObject::__setattr__;
	newtype->tp_setattro = 0;
    newtype->tp_dictoffset = offsetof_nw(HybridObject, m_dict);
	newtype->tp_base = baseClass;
	newtype->tp_dict = PyDict_Copy(dict);
	Py_XINCREF(baseClass);
    return (PyObject*)newtype;
}



} // end of namespace Robin::Python

} // end of namespace Robin
