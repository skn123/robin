/*
 * wrappedRobinType.cc
 *
 *  Created on: Jan 19, 2010
 *      Author: Marcelo Taube
 */
#include <Python.h>
#include <robin/frontends/frontend.h>
#include <robin/frontends/framework.h>
#include "wrappedrobintype.h"

#include "pythonfrontend.h"
namespace Robin {

namespace Python {

PyTypeObject WrappedRobinTypeTypeObject = {
	PyObject_HEAD_INIT(NULL)
    0,
    "robin.RobinType",
    sizeof( WrappedRobinType),
    0,
    RobinPyObject::__dealloc__, /*tp_dealloc*/
    0,                           /*tp_print*/
    WrappedRobinType::__getattr__, /*tp_getattr*/
    0, /*tp_setattr*/
    0,                           /*tp_compare*/
    WrappedRobinType::__repr__,    /*tp_repr*/
    0,                           /*tp_as_number*/
    0,                           /*tp_as_sequence*/
    0,                           /*tp_as_mapping*/
    0,                           /*tp_hash */
    0,     /*tp_call*/
    0,			/* tp_str */
	0,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,	/* tp_flags */
	"A RobinType represents the type of an objects like a python type, "
	"but types in robin can be more specific.\n"
	"For example the python type of an object can be list but the robin type"
	"of an object might be 'list of integers'.",	/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	0,				/* tp_methods */
	0,					/* tp_members */
	0,				/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	WrappedRobinType::pythonConstruct				/* tp_new */
};

WrappedRobinType::WrappedRobinType(Handle<RobinType> underlying)
	: RobinPyObject(&WrappedRobinTypeTypeObject), m_underlying(underlying)
{

}
PyReferenceSteal<WrappedRobinType> WrappedRobinType::construct(Handle<RobinType> underlying)
{
	WrappedRobinType *wrappedRobinType = new WrappedRobinType(underlying);
	return PyReferenceSteal<WrappedRobinType>(wrappedRobinType);
}

PyObject *WrappedRobinType::pythonConstruct(PyTypeObject *self, PyObject *args, PyObject *kwargs )
{
    static char *kwlist[] = {"type", NULL};
    PyObject *type;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:RobinType", kwlist,
                                         &type))
    {
            return NULL;
    }
    if(PyObject_TypeCheck(type, &WrappedRobinTypeTypeObject)) {
    	// If a WrappedRobinType object was passed, simply
    	// return it
    	Py_XINCREF(type);
    	return type;
    }
    if(!PyType_Check(type)){
    	PyErr_SetString(PyExc_TypeError,"The 'type' argument for RobinType must be a type");
    	return NULL;
    }
    Handle<RobinType> robinType = ((PythonFrontend*)FrontendsFramework::activeFrontend())->detectType((PyTypeObject*)type);
    PyReferenceSteal<WrappedRobinType> wrappedRobinType = construct(robinType);
    return wrappedRobinType.release();
}


WrappedRobinType::~WrappedRobinType()
{

}

PyObject *WrappedRobinType::__repr__(PyObject * self) {
	PyReferenceCreate<WrappedRobinType> self_ref((WrappedRobinType*)self);
	return self_ref->__repr__();
}

PyObject *WrappedRobinType::__repr__()
{
	using std::string;
	string name = string("<RobinType ") + m_underlying->getTypeName() + " >";
	return PyString_FromStringAndSize(name.c_str(),name.size());
}

/**
 * (static) Getattr protocol - invokes getattr on 'self', which is assumed to
 * be a function.
 */
PyObject *WrappedRobinType::__getattr__(PyObject *self, char *name)
{
	return ((WrappedRobinType*)self)->__getattr__(name);
}

/**
 * Provides specific class attributes.
 * <ul>
 *   <li>is_const = whatever this is a constant reference type</li>
 * </ul>
 */
PyObject *WrappedRobinType::__getattr__(const char *name)
{
	if (strcmp(name, "is_const") == 0) {

		if((m_underlying->isConstant() == RobinType::constReferenceKind)) {
			Py_XINCREF(Py_True);
			return Py_True;
		} else {
			Py_XINCREF(Py_False);
			return Py_False;
		}
	} else {
		PyErr_SetString(PyExc_AttributeError, name);
		return 0;
	}
}

} //end of namespace Robin::Python
} //end of namespace Robin
