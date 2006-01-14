// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#include "inheritance.h"

#include "pythonobjects.h"
#include "../../reflection/class.h"
#include "../../reflection/instance.h"


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
    return new HybridObject(classtype);
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

PyObject *HybridObject::__init__(PyObject *self, PyObject *args)
{
	ClassObject *klass = (ClassObject*)(self->ob_type);

	Handle<Instance> constructed_value = klass->construct(args, NULL);
	if (constructed_value) {
		((HybridObject*)self)->init(constructed_value);
		Py_XINCREF(Py_None); return Py_None;
	}
	else
		return NULL;
}

/**
 * Hybrid object attribute access:
 */
PyObject *HybridObject::__getattr__(char *nm)
{
	static PyMethodDef methods[] = {
		{ "init", __init__, METH_VARARGS, "initializes C instance" },
		{ 0 }
	};

	PyObject *value;

	if (value = PyDict_GetItemString(ob_type->tp_dict, nm)) {
		return PyMethod_New(value, this, PyObject_Type(this));
	}
	else {
		PyErr_Clear();
		if (value = Py_FindMethod(methods, this, nm)) {
			return value;
		}
		else {
			PyErr_Clear();
			if (value = PyDict_GetItemString(m_dict, nm)) {
				Py_XINCREF(value);
				return value;
			}
			else {
				PyErr_Clear();
				return InstanceObject::__getattr__(nm);
			}
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
