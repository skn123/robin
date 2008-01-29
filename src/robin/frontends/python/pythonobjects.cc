// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/python/pythonobjects.cc
 *
 * @par TITLE
 * Python Frontend Objects
 *
 * @par PACKAGE
 * Robin
 */

#include "pythonobjects.h"

// Robin includes
#include <robin/reflection/cfunction.h>
#include <robin/reflection/class.h>
#include <robin/reflection/instance.h>
#include <robin/reflection/address.h>
#include <robin/reflection/enumeratedtype.h>
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/frontends/framework.h>

// Python frontend includes
#include "enhancements.h"
#include "pythonadapters.h"
#include "pythonfrontend.h"
#include "pythonerrorhandler.h"
#include "inheritance.h"

namespace {

	long bwor(long a, long b) { return a | b; }
	long bwxor(long a, long b) { return a ^ b; }
	long bwand(long a, long b) { return a & b; }

	int dummy_coerce(PyObject **v, PyObject **w)
	{
		Py_XINCREF(*v);
		Py_XINCREF(*w);
		return 0;
	}

	PyObject *pyowned(PyObject *o) { Py_XINCREF(o); return o; }

#define offsetof_nw(CLASS, FIELD) ((char*)&((CLASS*)1)->FIELD - (char*)1)

}


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

PyTypeObject FunctionTypeObject = {
	PyObject_HEAD_INIT(&PyType_Type)
    0,
    "Robin::Python::FunctionObject",
    sizeof(FunctionObject),
    0,
    FunctionObject::__dealloc__, /*tp_dealloc*/
    0,                           /*tp_print*/
    FunctionObject::__getattr__, /*tp_getattr*/
    0,                           /*tp_setattr*/
    0,                           /*tp_compare*/
	FunctionObject::__repr__,    /*tp_repr*/
    0,                           /*tp_as_number*/
    0,                           /*tp_as_sequence*/
    0,                           /*tp_as_mapping*/
    0,                           /*tp_hash */
	FunctionObject::__call__     /*tp_call*/
};

#if 0
PyTypeObject ClassTypeObject = {
	PyObject_HEAD_INIT(&PyType_Type)
    0,
    "Robin::Python::ClassObject",
    sizeof(ClassObject),
    0,
    ClassObject::__dealloc__,    /*tp_dealloc*/
    0,                           /*tp_print*/
    ClassObject::__getattr__,    /*tp_getattr*/
    ClassObject::__setattr__,    /*tp_setattr*/
    0,                           /*tp_compare*/
    ClassObject::__repr__,       /*tp_repr*/
    0,                           /*tp_as_number*/
    0,                           /*tp_as_sequence*/
    0,                           /*tp_as_mapping*/
    PyType_Type.tp_hash,         /*tp_hash */
	ClassObject::__call__,       /*tp_call*/
	0,                                    /*tp_str*/
	0,                                    /*tp_getattro*/
	0,                                    /*tp_setattro*/
	0,                                    /*tp_as_buffer*/
	Py_TPFLAGS_BASETYPE | 
	Py_TPFLAGS_HAVE_CLASS,                /*tp_flags*/
	"",                                   /*tp_doc*/
	0,                                    /*tp_traverse*/
	0,                                    /*tp_clear*/
	0,                                    /*tp_richcompare*/
	0, /* tp_weakoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	0, /* tp_methods */
	0, /* tp_members */
	0, /* tp_getset */
	&PyType_Type, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */ 
	0, /* tp_alloc */
	0, /* tp_new */
	0, 0, 
	0
};
#endif

PyTypeObject *ClassTypeObject = 0;

PyTypeObject AddressTypeObject = {
	PyObject_HEAD_INIT(&PyType_Type)
    0,
    "Robin::Python::AddressObject",
    sizeof(AddressObject),
    0,
    AddressObject::__dealloc__,           /*tp_dealloc*/
    0,                                    /*tp_print*/
    0,                                    /*tp_getattr*/
    0,                                    /*tp_setattr*/
    0,                                    /*tp_compare*/
    AddressObject::__repr__,              /*tp_repr*/
    0,                                    /*tp_as_number*/
    0,                                    /*tp_as_sequence*/
    0,                                    /*tp_as_mapping*/
    PyType_Type.tp_hash,                  /*tp_hash */
	0,                                    /*tp_call*/
	0,                                    /*tp_str*/
	0,                                    /*tp_getattro*/
	0,                                    /*tp_setattro*/
	0,                                    /*tp_as_buffer*/
	Py_TPFLAGS_BASETYPE | 
	Py_TPFLAGS_HAVE_CLASS,                /*tp_flags*/
	"",                                   /*tp_doc*/
	0,                                    /*tp_traverse*/
	0,                                    /*tp_clear*/
	0,                                    /*tp_richcompare*/
	0, /* tp_weakoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	0, /* tp_methods */
	0, /* tp_members */
	0, /* tp_getset */
	&PyBaseObject_Type, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */ 
	0, /* tp_alloc */
	0, /* tp_new */
	0, 0, 
	0
};

PyTypeObject EnumeratedTypeTypeObject = {
	PyObject_HEAD_INIT(&PyType_Type)
    0,
    "Robin::Python::EnumeratedTypeObject",
    sizeof(ClassObject),
    0,
    EnumeratedTypeObject::__dealloc__,    /*tp_dealloc*/
    0,                                    /*tp_print*/
    0,                                    /*tp_getattr*/
    0,                                    /*tp_setattr*/
    0,                                    /*tp_compare*/
    EnumeratedTypeObject::__repr__,       /*tp_repr*/
    0,                                    /*tp_as_number*/
    0,                                    /*tp_as_sequence*/
    0,                                    /*tp_as_mapping*/
    PyType_Type.tp_hash,                  /*tp_hash */
	EnumeratedTypeObject::__call__,       /*tp_call*/
	0,                                    /*tp_str*/
	0,                                    /*tp_getattro*/
	0,                                    /*tp_setattro*/
	0,                                    /*tp_as_buffer*/
	Py_TPFLAGS_BASETYPE | 
	Py_TPFLAGS_HAVE_CLASS,                /*tp_flags*/
	"",                                   /*tp_doc*/
	0,                                    /*tp_traverse*/
	0,                                    /*tp_clear*/
	0,                                    /*tp_richcompare*/
	0, /* tp_weakoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	0, /* tp_methods */
	0, /* tp_members */
	0, /* tp_getset */
	&PyType_Type, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */ 
	0, /* tp_alloc */
	0, /* tp_new */
	0, 0, 
	0
};

PyNumberMethods EnumeratedConstantNumberMethods = {
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 
	EnumeratedConstantObject::__and__,
	EnumeratedConstantObject::__xor__, 
	EnumeratedConstantObject::__or__, 
	dummy_coerce,
	EnumeratedConstantObject::__int__
};


PyMappingMethods ConversionHookMappingMethods = {
	0,                                     /*mp_length*/
	0,                                     /*mp_subscript*/
	ConversionHookObject::__setsubscript__ /*mp_ass_subscript*/
};

PyTypeObject ConversionHookTypeObject = {
	PyObject_HEAD_INIT(NULL)
    0,
    "Robin::Python::ConversionHookObject",
    sizeof(ConversionHookObject),
    0,
    ConversionHookObject::__dealloc__,    /*tp_dealloc*/
    0,                                    /*tp_print*/
    0,                                    /*tp_getattr*/
    0,                                    /*tp_setattr*/
    0,                                    /*tp_compare*/
    0,                                    /*tp_repr*/
    0,                                    /*tp_as_number*/
    0,                                    /*tp_as_sequence*/
    &ConversionHookMappingMethods,        /*tp_as_mapping*/
    0,                                    /*tp_hash */
	0,                                    /*tp_call*/
};


namespace {

	Handle<PyCallableWithInstance> pycallableFactory(PyObject *value)
	{
		if (PyString_Check(value)) {
			// Create an instance-method-call functor
			std::string methodname = PyString_AsString(value);
			return Handle<PyCallableWithInstance>
				(new InstanceMethodFunctor(methodname));
		}
		else if (PyCallable_Check(value)) {
			// Create a python-object-native-call functor
			return Handle<PyCallableWithInstance>
				(new PyObjectNativeFunctor(value));
		}
		else if (value == Py_None) {
			// Set handler to a null Handle<>
		}
		else {
			PyErr_SetString(PyExc_TypeError,
							"expected: string, function, or None");
		}
		return Handle<PyCallableWithInstance>();
	}

	/**
	 * Stores a reference to 'self' in 'keeper'.
	 *
	 * @param self object to keep reference to
	 * @param keeper an instance object which will hold the reference.
	 * If 'keeper' is not an InstanceObject, the operation fails silently.
	 */
	void keepMeAlive(PyObject *self, PyObject *keeper)
	{
		if (InstanceObject_Check(keeper)) {
			((InstanceObject*)keeper)->keepAlive(self);
		}
	}

}



/**
 * FunctionObject constructor.
 */
FunctionObject::FunctionObject(Handle<Callable> underlying)
	: m_underlying(underlying), m_self(NULL), m_in_module(NULL)
{
	PyObject_Init(this, &FunctionTypeObject);
}

/**
 * FunctionObject constructor.
 */
FunctionObject::FunctionObject(Handle<CallableWithInstance> underlying,
							   InstanceObject *self)
	: m_underlying_thiscall(underlying), m_self(self), m_in_module(NULL)
{
	PyObject_Init(this, &FunctionTypeObject);
	Py_XINCREF(self);
}

/**
 * FunctionObject destructor.
 */
FunctionObject::~FunctionObject()
{
	Py_XDECREF(m_self);
	Py_XDECREF(m_in_module);
}

void FunctionObject::setName(const std::string& name)
{
	m_name = name;
}

/**
 * Associate an instance object with this function object. If the function is
 * global or static, self == NULL. Otherwise it may point to an existing
 * instance object corresponding to the C++ instance upon which the method was
 * invoked. The FunctionObject then holds an actual owned reference to the
 * instance for the short time during which it lives.
 */
void FunctionObject::setSelf(InstanceObject *self)
{
	Py_XDECREF(m_self);
	m_self = self;
	Py_XINCREF(m_self);
}

/**
 * Tell the function which module contains it, so the FunctionObject may
 * return this value as __module__.
 */
void FunctionObject::inModule(PyObject *module)
{
	Py_XDECREF(m_in_module);
	Py_XINCREF(module);
	m_in_module = module;
}

/**
 * Deallocator - called by Python when the object is destroyed.
 */
void FunctionObject::__dealloc__(PyObject *self)
{
	delete (FunctionObject*)self;
}

/**
 * (static) Call protocol - invokes call on 'self', which is assumed to be
 * a function, with given arguments.
 */
PyObject *FunctionObject::__call__(PyObject *self, PyObject *args, 
								   PyObject *kw)
{
	// Perform the call
	return ((FunctionObject*)self)->__call__(args, kw);
}

/**
 * Invokes call on current object with given arguments.
 */
PyObject *FunctionObject::__call__(PyObject *args, PyObject *kw)
{
	// Construct arguments
	int nargs = PyTuple_Size(args);
	ActualArgumentList pass_args(nargs);
	for (int argindex = 0; argindex < nargs; ++argindex)
		pass_args[argindex] = PyTuple_GetItem(args, argindex);


    // XXX: initialize from kw
    KeywordArgumentMap kwargs;

    if(kw != NULL && kw != Py_None) {
        assert(PyDict_Check(kw));
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while(PyDict_Next(kw, &pos, &key, &value)) {
            std::string kwmap_key(PyString_AsString(key));
            kwargs[kwmap_key] = value;
        }
    }

	// Invoke callable item
	scripting_element return_value;
	try {
		if (m_self)
			return_value = m_underlying_thiscall->
				callUpon(*m_self->getUnderlying(), pass_args, kwargs, m_self);
		else
			return_value = m_underlying->call(pass_args, kwargs);
	}
	catch (const UserExceptionOccurredException& e) {
		// - retrieve the current error information, if possible
		PyObject *errinfo = (PyObject*)
			FrontendsFramework::activeFrontend()->
				getErrorHandler().getError();
		FrontendsFramework::activeFrontend()->getErrorHandler().setError(NULL);
		if (errinfo) {
			// - restore previous error information
			PyObject *exc_type, *exc_value, *exc_tb;
			PyArg_ParseTuple(errinfo, "OOO", &exc_type, &exc_value, &exc_tb);
			PyErr_Restore(exc_type, exc_value, exc_tb);
			return NULL;
		}
		else {
			// - translate exception to Python
			PyErr_SetString(PyExc_RuntimeError, e.user_what.c_str());
			return NULL;
		}
	}
	catch (const std::exception& e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
		return NULL;
	}

	// Successful completion
	if (return_value == NONE) {
		return pyowned(Py_None);
	}
	else {
		return (PyObject*)return_value;
	}
}

/**
 * (static) Getattr protocol - invokes getattr on 'self', which is assumed to
 * be a function.
 */
PyObject *FunctionObject::__getattr__(PyObject *self, char *name)
{
	return ((FunctionObject*)self)->__getattr__(name);
}

/**
 * Provides specific class attributes.
 * <ul>
 *   <li>__name__ = function name</li>
 *   <li>__self__ = the instance upon which the function is called, if this
 *     is an instance method. Otherwise, None.</li>
 * </ul>
 */
PyObject *FunctionObject::__getattr__(const char *name)
{
	if (strcmp(name, "__self__") == 0) {
		PyObject *ret = m_self ? m_self : Py_None;
		return pyowned(ret);
	}
	else if (strcmp(name, "__module__") == 0) {
		if (m_in_module)
			return PyString_FromString(PyModule_GetName(m_in_module));
		else if (m_self)
			// get the module from the instance object containing this function
			return PyObject_GetAttrString(m_self, "__module__");
		else
			return pyowned(Py_None);
	}
	else if (strcmp(name, "__name__") == 0) {
		return PyString_FromString(m_name.c_str());
	}
	else {
		PyErr_SetString(PyExc_AttributeError, name);
		return 0;
	}
}

/**
 * (static) Repr protocol function - invokes __setattr__.
 */
PyObject *FunctionObject::__repr__(PyObject *self)
{
    return ((FunctionObject*)self)->__repr__();
}

/**
 * Produces a textual representation of the function object as:
 * <code>"&lt;RobinFunction 'functionname'&gt;"</code>
 */
PyObject *FunctionObject::__repr__()
{
    const char *format = "<RobinFunction '%s'>";
    std::string name = m_name;
    // Add the full name of the container class to the function's name
    if (m_self) {
		name = m_self->getUnderlying()->getClass()->name() + "::" + name;
    }
    // Format the string
    char *buffer = new char[strlen(format) + name.size()];
    sprintf(buffer, format, name.c_str());
    // Return string to Python
    PyObject *fmt = PyString_FromString(buffer);
    delete[] buffer;
    return fmt;
}

/**
 * ClassObject constructor.
 */
ClassObject::ClassObject(Handle<Class> underlying)
	: m_underlying(underlying), m_in_module(NULL), m_x_methods(NULL)
{
	(PyTypeObject&)(*this) = NullTypeObject;

	PyObject_Init((PyObject*)this, ClassTypeObject);

	ob_size = sizeof(ClassObject);

	char *new_name = (char *)malloc(underlying->name().size() + 1);
	strcpy(new_name, underlying->name().c_str());
	tp_name = new_name;
	tp_basicsize = sizeof(InstanceObject);
	tp_dealloc = InstanceObject::__dealloc__;
	tp_print = NULL;
	tp_getattr = InstanceObject::__getattr__;
	tp_setattr = InstanceObject::__setattr__;
	tp_compare = NULL;
	tp_repr = InstanceObject::__repr__;

	tp_as_number = NULL;
	tp_as_sequence = NULL;
	tp_as_mapping = NULL;

	tp_hash = NULL;
	tp_call = NULL;
	tp_str = NULL;
	tp_getattro = NULL;
	tp_setattro = NULL;

	tp_as_buffer = NULL;

	// Support the new class model
	tp_flags = Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_CLASS | 
		       Py_TPFLAGS_CHECKTYPES | Py_TPFLAGS_HAVE_WEAKREFS;
	tp_base = &PyBaseObject_Type;

	tp_mro = NULL;
	tp_new = __new__;
	tp_weaklistoffset = offsetof_nw(InstanceObject, m_ob_weak);

	m_inners = PyDict_New();
}

ClassObject::~ClassObject()
{
	if (m_x_methods) x_releaseMethodTable();
	Py_XDECREF(m_in_module);
}

/**
 * Initializes a C instance.
 */
PyObject *ClassObject::__init__(PyObject *self, PyObject *args)
{
	ClassObject *klass = (ClassObject*)(self->ob_type);

	Handle<Instance> constructed_value = klass->construct(args, NULL);
	if (constructed_value) {
		((InstanceObject*)self)->init(constructed_value);
		Py_XINCREF(Py_None); return Py_None;
	}
	else
		return NULL;
}

/**
 * External initialization (i.e. with self as first argument)
 */
PyObject *ClassObject::__init_ex__(PyObject *, PyObject *self_and_args)
{
	assert(PyTuple_Check(self_and_args));
	assert(PyTuple_Size(self_and_args) >= 1);
	PyObject *self = PyTuple_GetItem(self_and_args, 0);
	PyObject *args = PyTuple_GetSlice(self_and_args, 1, 
									  PyTuple_Size(self_and_args));
	PyObject *ret = __init__(self, args);
	Py_XDECREF(args);
	return ret;
}

/**
 */
PyObject *ClassObject::__new__(PyTypeObject *self, PyObject *args,
							   PyObject *kw)
{
	return __call__((PyObject*)self, args, kw);
}

/**
 */
PyObject *ClassObject::__call__(PyObject *self, PyObject *args,
								PyObject *kw)
{
	return ((ClassObject*)self)->__call__(args, kw);
}

/**
 * Implements the call protocol: Creates an instance of this class with
 * given constructor arguments.
 */
PyObject *ClassObject::__call__(PyObject *args, PyObject *kw)
{
	Handle<Instance> constructed_value = construct(args, kw);
	if (constructed_value)
		return (PyObject *)new InstanceObject(this, constructed_value);
	else
		return NULL;
}

Handle<Instance> ClassObject::construct(PyObject *args, PyObject *kw)
{
	// Construct arguments
	int nargs = PyTuple_Size(args);
	ActualArgumentList pass_args(nargs);
	for (int argindex = 0; argindex < nargs; ++argindex)
		pass_args[argindex] = PyTuple_GetItem(args, argindex);


    // XXX: Initialize from kw
    KeywordArgumentMap kwargs;

	// Invoke callable item
	try {
		return m_underlying->createInstance(pass_args, kwargs);
	}
	catch (const UserExceptionOccurredException& e) {
		// - retrieve the current error information, if possible
		PyObject *errinfo = (PyObject*)
			FrontendsFramework::activeFrontend()->
				getErrorHandler().getError();
		FrontendsFramework::activeFrontend()->getErrorHandler().setError(NULL);
		if (errinfo) {
			// - restore previous error information
			PyObject *exc_type, *exc_value, *exc_tb;
			PyArg_ParseTuple(errinfo, "OOO", &exc_type, &exc_value, &exc_tb);
			PyErr_Restore(exc_type, exc_value, exc_tb);
		}
		else {
			// - translate exception to Python
			PyErr_SetString(PyExc_RuntimeError, e.user_what.c_str());
		}
	}
	catch (const std::exception& e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
	}
	return Handle<Instance>();
}

/**
 * (static) Get-attribute protocol function - invokes __getattr__.
 */
PyObject *ClassObject::__getattr__(PyObject *self, char *name)
{
	return ((ClassObject*)self)->__getattr__(name);
}

/**
 * Provides specific class attributes.
 * <ul>
 *   <li>__name__ = class name as string</li>
 *   <li>__from__ = a ConversionHookObject via which you can assign conversions
 *       to this class type</li>
 * </ul>
 */
PyObject *ClassObject::__getattr__(const char *name)
{
	if (strcmp(name, "__name__") == 0) {
		return PyString_FromString(getUnderlying()->name().c_str());
	}
	else if (strcmp(name, "__from__") == 0) {
		return new ConversionHookObject(getUnderlying());
	}
	else if (strcmp(name, "__from_volatile__") == 0) {
		return new ConversionHookObject(getUnderlying(),
										ConversionHookObject::VOLATILE);
	}
	else if (strcmp(name, "__module__") == 0) {
		if (m_in_module)
			return PyString_FromString(PyModule_GetName(m_in_module));
		else
			return pyowned(Py_None);
	}
	else if (strcmp(name, "__dict__") == 0) {
		PyObject* dict = this->getDict();
		Py_INCREF(dict);
		return dict;
	}
    // return C++ instance __init__ only if the tp_dict doesn't have __init__
    // already
	else if (strcmp(name, "__init__") == 0 && (tp_dict == NULL ||
                !PyDict_GetItemString(tp_dict, (char*)name))) {
		static PyMethodDef method = {
			"__init__", 
			__init_ex__, 
			METH_VARARGS | METH_CLASS, 
			"initializes C instance" 
		};
		return PyMethod_New(PyCFunction_New(&method,0), NULL, (PyObject*)this);
	}
	else if (strcmp(name, "__bases__") == 0) {
		return Py_BuildValue("(O)", &PyType_Type);
	}
	else {

        if(tp_dict) {
            // look in the dict - this is for a case where our ClassObject is
            // inherited by python classes
            PyObject* dictmember = PyDict_GetItemString(tp_dict, (char*)name);
            if(dictmember) {
                if(PyMethod_Check(dictmember) || PyFunction_Check(dictmember)) {
                    return PyMethod_New(dictmember, NULL, (PyObject*)this); 
                } else {
                    Py_XINCREF(dictmember);
                    return dictmember;
                }
            } else {
                PyErr_Clear();
            }
        }
		// - look for static inners
		PyObject *inner = PyDict_GetItemString(m_inners, (char*)name);
		if (inner) {
			return pyowned(inner);
		}
		else {
			PyErr_SetString(PyExc_AttributeError, name);
			return NULL;
		}
	}
}

/**
 * (static) Set-attribute protocol function - invokes __setattr__.
 */
int ClassObject::__setattr__(PyObject *self, char *name, PyObject *value)
{
	return ((ClassObject*)self)->__setattr__(name, value);
}

/**
 * Assigns a protocol handler to a slot by name. The handler may be one of
 * <ul><li>string - specifying an instance method to be invoked</li>
 *     <li>function - specifying a Python function to be invoked</li>
 *     <li>None - to disable this slot</li>
 * </ul>
 * For example, to set the GETITEM protocol to invoke the instance method
 * "get":
 * <p><code>Class.__getitem__ = "get"</code></p>
 * <p>In addition, if the __to__ conversion protocol is being set, the input
 * can be a tuple containing any of the above mentioned handlers, and a
 * number representing the weight of the conversion.</p>
 */
int ClassObject::__setattr__(char *name, PyObject *value)
{
	Handle<PyCallableWithInstance> handler;

	try {
		handler = pycallableFactory(value);
		if (PyErr_Occurred()) return -1;

		if (strcmp(name, "__to__") == 0) {
			// Register an edge conversion
			Handle<Conversion> ptr_exit(new PythonConversion(handler));
			Handle<Conversion> ref_exit(new PythonConversion(handler));
			ptr_exit->setSourceType(getUnderlying()->getPtrArg());
			ref_exit->setSourceType(getUnderlying()->getRefArg());
			ConversionTableSingleton::getInstance()
				->registerEdgeConversion(ptr_exit);
			ConversionTableSingleton::getInstance()
				->registerEdgeConversion(ref_exit);
		}
		else {
			// Deploy functor as handler for requested protocol
			EnhancementsPack::SlotID slot = EnhancementsPack::slotByName(name);
			Protocol::deployerBySlotName(name).deploy(this, slot, handler);
		}
		return 0;
	}
	catch (const EnhancementsPack::NoSuchSlotException& ) {
		PyErr_SetString(PyExc_AttributeError, "invalid slot name for class "
						"enhancement.");
		return -1;
	}
}

/**
 * (static) Repr protocol function - invokes __setattr__.
 */
PyObject *ClassObject::__repr__(PyObject *self)
{
	return ((ClassObject*)self)->__repr__();
}

/**
 * Produces a textual representation of the class object as:
 * <code>"&lt;Robin::Python::ClassObject 'classname'&gt;"</code>
 */
PyObject *ClassObject::__repr__()
{
	const char *format = "<Robin class '%s'>";
	std::string name = getUnderlying()->name();
	// Format the string
	char *buffer = new char[strlen(format) + name.size()];
	sprintf(buffer, format, name.c_str());
	// Return string to Python
	PyObject *fmt = PyString_FromString(buffer);
	delete[] buffer;
	return fmt;
}

/**
 * Deallocator - called by Python when the object is destroyed.
 */
void ClassObject::__dealloc__(PyObject *self)
{
	delete (ClassObject*)self;
}

/**
 * Tell the class which module contains it, so the ClassObject may return
 * the value of __module__.
 */
void ClassObject::inModule(PyObject *module)
{
	Py_XDECREF(m_in_module);
	Py_XINCREF(module);
	m_in_module = module;
}

/**
 * Returns the underlying Robin::Class object from the reflection.
 */
Handle<Class> ClassObject::getUnderlying() const
{
	return m_underlying;
}

/**
 * Returns a reference to the Python module that contains this class.
 *
 * @return a borrowed reference to a Python module
 */
PyObject *ClassObject::getContainingModule() const
{
	return m_in_module;
}

/**
 * Returns a reference to a Python dictionary which contains static inner
 * members of this class:
 * <ul>
 *  <li>Inner classes</li>
 *  <li>Static methods</li>
 * </ul>
 *
 * @return a borrowed reference to a Python dictionary
 */
PyObject *ClassObject::getDict() const
{
	return m_inners;
}

/**
 * Looks up an instance method in this class.
 */
Handle<CallableWithInstance> ClassObject::
findInstanceMethod(const char *name, const char **internal_name) const
{
	if (m_x_methods == NULL) x_optimizeMethodTable();

	Handle<CallableWithInstance> method = x_findMethod(name, internal_name);
	if (method)
		return method;
	else
		throw NoSuchMethodException();
}

/**
 * Returns an enhancement cluster associated with this class object in
 * the Python interpreter.
 */
EnhancementsPack& ClassObject::getEnhancements()
{
	return m_enhance;
}

/**
 * Returns an enhancement cluster associated with this class object in
 * the Python interpreter (const version).
 */
const EnhancementsPack& ClassObject::getEnhancements() const
{
	return m_enhance;
}

/**
 * Creates a method table - an array of ClassObject::MethodDef structures
 * containing direct access to class methods. This speeds up search in
 * implementation of __getattr__.
 */
void ClassObject::x_optimizeMethodTable() const
{
	const std::string& DATAMEMBER_PREFIX = PythonFrontend::DATAMEMBER_PREFIX;
	// Get the names of the methods to put in table
	//     (note that inherited methods are transparently returned by
	//      listMethods()).
	std::vector<std::string> method_names = m_underlying->listConciseMethods();

	// Allocate the table
	m_x_methods = new MethodDef[method_names.size() + 1];

	// Fill table with information about each method by looking it up
	for (size_t methindex = 0; methindex < method_names.size(); ++methindex) {
		const char *name = strdup(method_names[methindex].c_str());
		// - data members are given their "real" name, ".data_" chopped
		if (0 == strncmp(name, DATAMEMBER_PREFIX.c_str(), DATAMEMBER_PREFIX.size()))
			m_x_methods[methindex].name = name + DATAMEMBER_PREFIX.size();
		else
			m_x_methods[methindex].name = name;
		// - the internal name holds this method's Robin-given name
		m_x_methods[methindex].internal_name = name;
		// - a handle to the method's implementation is associated
		m_x_methods[methindex].meth = 
			m_underlying->findInstanceMethod(method_names[methindex]);
	}

	// Sentinel
	m_x_methods[method_names.size()].name = NULL;
}

Handle<CallableWithInstance> ClassObject::
x_findMethod(const char *name, const char **internal_name) const
{
	for (MethodDef *def = m_x_methods; def->name; ++def) {
		if (name[0] == def->name[0] && strcmp(name, def->name) == 0) {
			*internal_name = def->internal_name;
			return def->meth;
		}
	}

	return Handle<CallableWithInstance>();
}

/**
 * Frees space consumed by the method table.
 */
void ClassObject::x_releaseMethodTable()
{
	for (MethodDef *def = m_x_methods; def->name; ++def) {
		free((void*)def->internal_name);
	}
	delete[] m_x_methods;
	m_x_methods = 0;
}

/**
 * InstanceObject constructor.
 */
InstanceObject::InstanceObject(ClassObject *classobj, 
							   Handle<Instance> underlying)
	: m_underlying(underlying), m_ownership_keep(0), m_ob_weak(0)
{
	PyObject_Init(this, classobj);
}

InstanceObject::InstanceObject() 
	: m_ownership_keep(0), m_ob_weak(0)
{
}

void InstanceObject::init(Handle<Instance> underlying)
{
	assert(!m_underlying);
	m_underlying = underlying;
}

InstanceObject::~InstanceObject()
{
	if (m_ownership_keep) {
		Py_INCREF(this);
		Py_XDECREF(m_ownership_keep);
	}
	if (m_ob_weak != NULL)
		PyObject_ClearWeakRefs((PyObject *) this);
}

/**
 * Deallocator - called by Python when the object is destroyed.
 */
void InstanceObject::__dealloc__(PyObject *self)
{
	delete (InstanceObject*)self;
}

/**
 * Checks whether this object has been initialized either via constructor
 * or using the init() method.
 */
bool InstanceObject::isInitialized() const
{
	return m_underlying;
}

/**
 * Accessor - returns underlying instance object.
 */
Handle<Instance> InstanceObject::getUnderlying() const
{
	assert(m_underlying);
	return m_underlying;
}

/**
 * Makes sure 'owner' is not destroyed before 'this', by incrementing
 * the reference count of 'owner'. The reference count is decremented when
 * 'this' is itself destroyed.
 *
 * @param owner Python object to keep alive
 */
void InstanceObject::keepAlive(PyObject *owner)
{
    Py_XDECREF(m_ownership_keep);
	m_ownership_keep = (owner == this)? NULL : owner;
	Py_XINCREF(m_ownership_keep);
}

/**
 * (static) Get-attribute protocol - invokes getattr on self, which is
 * assumed to be an instance object.
 */
PyObject *InstanceObject::__getattr__(PyObject *self, char *attrname)
{
	return ((InstanceObject*)self)->__getattr__(attrname);
}

/**
 * Implements the get-attribute protocol: looks for instance methods or
 * instance data members.
 */
PyObject *InstanceObject::__getattr__(const char *attrname)
{
	if (strcmp(attrname, "__methods__") == 0) {
		std::vector<std::string> methodNames =
			getUnderlying()->getClass()->listConciseMethods();

		PyObject *methodsList = PyList_New(0);
		for (size_t i = 0; i < methodNames.size(); ++i) {
			const char *current = methodNames[i].c_str();
			if (strncmp(current, PythonFrontend::DATAMEMBER_PREFIX.c_str(),
						PythonFrontend::DATAMEMBER_PREFIX.size()) == 0) {
				                                   // data member
				current += PythonFrontend::DATAMEMBER_PREFIX.size();
			}
			if (current[0] != '.')                 // hidden method
				PyList_Append(methodsList,
							  PyString_FromString(current));
		}
		return methodsList;
	}
	else if (strcmp(attrname, "__module__") == 0) {
		// get the module from the class object
		return PyObject_GetAttrString(PyObject_Type(this), "__module__");
	}
	// - "__" prefix is reserved for slots
	else if (attrname[0] == '_' && attrname[1] == '_') {
		EnhancementsPack::SlotID slot =
			EnhancementsPack::slotByName(attrname);
		if (slot == EnhancementsPack::NSLOTS) {
			PyErr_SetString(PyExc_AttributeError, "unrecognized slot");
			return NULL;
		}
		else {
			return PyInt_FromLong((long)slot);  // @@@ ?
		}
	}
	else {
		return getBoundMethodOrDataMember(attrname);
	}
}

/**
 * (static) Set-attribute protocol - invokes setattr on self, which is
 * assumed to be an instance object.
 */
int InstanceObject::__setattr__(PyObject *self, char *name, PyObject *val)
{
	return ((InstanceObject*)self)->__setattr__(name, val);
}

/**
 * Implements the set-attribute protocol; provides the syntax i.__owner__ = o
 * which causes o to be kept alive as long as i is.
 * Also, allows setting of internal members of the class via i.varname = val
 * syntax.
 */
int InstanceObject::__setattr__(char *name, PyObject *val)
{
    if(val == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Can't delete native attributes");
        return -1;
    }

	if (strcmp(name, "__owner__") == 0) {
		// - set the owner
		keepAlive(val);
		return 0;
	}
	else {
		const std::string &SINKMEMBER_PREFIX = PythonFrontend::SINKMEMBER_PREFIX;
		try {
			Handle<CallableWithInstance> meth =
                findFieldWrapper(SINKMEMBER_PREFIX, name); 

			// - data member setter
			ActualArgumentList args;
			args.push_back(val);

            KeywordArgumentMap nokwargs;
			meth->callUpon(*m_underlying, args, nokwargs);
			return 0;
		}
		catch (const std::exception &e) {
			// - attribute error
			PyErr_SetString(PyExc_AttributeError,
			                (std::string(name) + ": " + e.what()).c_str());
			return -1;
		}
	}
}


Handle<CallableWithInstance> InstanceObject::findFieldWrapper(const std::string &prefix, const char* name) 
{

    std::string full_name = prefix + name;
    const char *internal_name;    
    Handle<CallableWithInstance> meth = 
        ((ClassObject*)ob_type)->findInstanceMethod(
            full_name.c_str(), &internal_name);
    if (!meth) throw NoSuchMethodException();

    return meth;

}
        

/**
 * Tries to find an instance method and bind it with the current instance.
 * Or, if a data member by that name exists, retrieves the value of that
 * member.
 */
PyObject *InstanceObject::getBoundMethodOrDataMember(const char *name)
{
	const char *internal_name;

	try {
		Handle<CallableWithInstance> meth = 
			((ClassObject*)ob_type)->findInstanceMethod(name, &internal_name);
		if (!meth) throw NoSuchMethodException();

		if (name[0] != '.' && internal_name[0] == '.') {
			// - data member
			ActualArgumentList noargs;
            KeywordArgumentMap nokwargs;
			PyObject *member = 
				(PyObject*)meth->callUpon(*m_underlying, noargs, nokwargs);
			keepMeAlive(this, member);
			return member;
		}
		else {
			// - instance method
			FunctionObject *boundobj = new FunctionObject(meth, this);
			boundobj->setName(name);
			return boundobj;
		}
	}
	catch (const std::exception& e) {
		PyErr_SetString(PyExc_AttributeError, e.what());
		return NULL;
	}
}

/**
 * (static) Repr protocol - invokes getattr on self, which is
 * assumed to be an instance object.
 */
PyObject *InstanceObject::__repr__(PyObject *self)
{
	return ((InstanceObject*)self)->__repr__();
}

/**
 * Generates a most basic textual representation of the instance object
 * of the form 
 * <code>"&lt;Robin::Python::InstanceObject of 'classname' at 0xXXXX&gt;"
 * </code>
 */
PyObject *InstanceObject::__repr__()
{
	const char *format = "<Instance of Robin class '%s'>";

    std::string classname;

    if(!isInitialized()) {
        classname = "NOT INITIALIZED";
    } else {
        classname = getUnderlying()->getClass()->name();
    }
    
	// Format the string
	char *buffer = new char[strlen(format) + classname.size() + 2 + 16];
	sprintf(buffer, format, classname.c_str());
	// Return string to Python
	PyObject *fmt = PyString_FromString(buffer);
	delete[] buffer;
	return fmt;	
}

/**
 * AddressObject constructor.
 *
 * @param underlying a typed C pointer represented by an Address object.
 */
AddressObject::AddressObject(Handle<Address> underlying)
	: m_underlying(underlying)
{
	PyObject_Init(this, &AddressTypeObject);
}

AddressObject::~AddressObject() 
{
}

PyObject *AddressObject::__repr__(PyObject *self)
{
	return ((AddressObject*)self)->__repr__();
}

PyObject *AddressObject::__repr__() const
{
	return PyString_FromString("<address>");
}

void AddressObject::__dealloc__(PyObject *self)
{
	delete ((AddressObject*)self);
}

Handle<Address> AddressObject::getUnderlying() const
{
	return m_underlying;
}


/**
 * EnumeratedTypeObject constructor.
 */
EnumeratedTypeObject::EnumeratedTypeObject(Handle<EnumeratedType> underlying)
	: m_underlying(underlying)
{
	PyObject_Init((PyObject*)this, &EnumeratedTypeTypeObject);

	ob_size = sizeof(EnumeratedTypeObject);

	char *new_name = (char *)malloc(underlying->name().size() + 1);
	strcpy(new_name, underlying->name().c_str());
	tp_name = new_name;
	tp_basicsize = sizeof(EnumeratedConstantObject);
	tp_dealloc = EnumeratedConstantObject::__dealloc__;
	tp_print = NULL;
	tp_getattr = NULL;
	tp_setattr = NULL;
	tp_compare = NULL;
	tp_repr = EnumeratedConstantObject::__repr__;
	tp_richcompare = EnumeratedConstantObject::__richcmp__;

	tp_as_number = &EnumeratedConstantNumberMethods;
	tp_as_sequence = NULL;
	tp_as_mapping = NULL;

	tp_hash = NULL;
	tp_call = NULL;
	tp_str = NULL;
	tp_getattro = NULL;
	tp_setattro = NULL;

	tp_as_buffer = NULL;

	// Support the new class model
	tp_flags = Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_CLASS | 
	           Py_TPFLAGS_CHECKTYPES | Py_TPFLAGS_HAVE_RICHCOMPARE;
	tp_base = &PyBaseObject_Type;

	tp_new = &EnumeratedTypeObject::__new__;
}

EnumeratedTypeObject::~EnumeratedTypeObject()
{
}

PyObject *EnumeratedTypeObject::__new__(PyObject *args, PyObject *kw)
{
	long value;

	if (!PyArg_ParseTuple(args, "i", &value)) return NULL;

	Handle<EnumeratedConstant> enumconst(new EnumeratedConstant(m_underlying,
																value));
	EnumeratedConstantObject *pyenumconst =
		new EnumeratedConstantObject(this, enumconst);

	return pyenumconst;
}

/**
 * Constructs a textual representation of the enumerated type having the
 * form:
 * <code>&lt;Robin enum 'enum-name'&gt;</code>
 */
PyObject *EnumeratedTypeObject::__repr__()
{
	const char *format = "<Robin enum '%s'>";
	std::string name = getUnderlying()->name();
	// Format the string
	char *buffer = new char[strlen(format) + name.size()];
	sprintf(buffer, format, name.c_str());
	// Return string to Python
	PyObject *fmt = PyString_FromString(buffer);
	delete[] buffer;
	return fmt;
}

Handle<EnumeratedType> EnumeratedTypeObject::getUnderlying() const
{
	return m_underlying;
}

/**
 * (static) New protocol function - invokes EnumeratedTypeObject::__new__
 */
PyObject *EnumeratedTypeObject::__new__(PyTypeObject *self, PyObject *args,
										PyObject *kw)
{
	return ((EnumeratedTypeObject *)self)->__new__(args, kw);
}

/**
 * (static) Call protocol function - invokes EnumeratedTypeObject::__call__
 */
PyObject *EnumeratedTypeObject::__call__(PyObject *self, PyObject *args,
										 PyObject *kw)
{
	return ((EnumeratedTypeObject *)self)->__new__(args, kw);
}

/**
 * (static) Repr protocol function - invokes EnumeratedTypeObject::__repr__
 */
PyObject *EnumeratedTypeObject::__repr__(PyObject *self)
{
	return ((EnumeratedTypeObject *)self)->__repr__();
}

void EnumeratedTypeObject::__dealloc__(PyObject *self)
{
	delete (EnumeratedTypeObject *)self;
}

/**
 * EnumeratedConstantObject constructor.
 */
EnumeratedConstantObject::EnumeratedConstantObject(EnumeratedTypeObject *
												   pytype,
												   Handle<EnumeratedConstant>
												   underlying)
	: m_underlying(underlying)
{
	PyObject_Init(this, pytype);
}

EnumeratedConstantObject::~EnumeratedConstantObject()
{
}

/**
 * (static) Deallocator - called by Python when object is destroyed.
 */
void EnumeratedConstantObject::__dealloc__(PyObject *self)
{
	delete ((EnumeratedConstantObject*)self);
}

/**
 * (static) Repr protocol function - invokes EnumeratedConstantObject::__repr__
 */
PyObject *EnumeratedConstantObject::__repr__(PyObject *self)
{
	return ((EnumeratedConstantObject*)self)->__repr__();
}

/**
 * Constructs a textual representation of the enumerated constant of the
 * form:
 * <code>(enum type)literal</code>
 */
PyObject *EnumeratedConstantObject::__repr__()
{
	std::string typedesc = getUnderlying()->getType()->name();
	std::string literal = getUnderlying()->getLiteral();
	std::string repr = "(enum " + typedesc + ")" + literal;
	return PyString_FromStringAndSize(repr.c_str(), int(repr.size()));
}

/**
 * (static) RichCmp protocol function - invokes
 *  EnumeratedConstantObject::__richcmp__
 */
PyObject *EnumeratedConstantObject::__richcmp__(PyObject *self,
												PyObject *other,
												int opid)
{
	PyObject *decision;

	// - only compare if objects are of the same type (both enums)
	if (PyObject_Type(PyObject_Type(other)) ==
		(PyObject*)&EnumeratedTypeTypeObject) {
		EnumeratedConstantObject *myself = (EnumeratedConstantObject*)self;
		if (myself->__richcmp__((EnumeratedConstantObject*)other, opid))
			decision = Py_True;
		else
			decision = Py_False;
	}
	else 
		decision = Py_False;

	Py_XINCREF(decision);
	return decision;
}

/**
 * Implements RichCmp by comparing the numeric value of the enumerated
 * constants. Only == and != are supported, trying to compare the order
 * will always yield false.
 */
bool EnumeratedConstantObject::__richcmp__(EnumeratedConstantObject *obj,
										   int opid)
{
	if (opid == Py_EQ)
		return (obj->m_underlying->getValue() == m_underlying->getValue());
	else if (opid == Py_NE)
		return (obj->m_underlying->getValue() != m_underlying->getValue());
	else
		return false;
}

/**
 * (static) Conversion to int.
 */
PyObject *EnumeratedConstantObject::__int__(PyObject *self)
{
	return ((EnumeratedConstantObject*)self)->__int__();
}

/**
 * Implements conversion to integer.
 */
PyObject *EnumeratedConstantObject::__int__() const
{
	return PyInt_FromLong(m_underlying->getValue());
}

/**
 * (static) Numeric AND protocol. Calls self->__aop__(other, ...) with
 * approperiate arguments for bitwise AND.
 */
PyObject *EnumeratedConstantObject::__and__(PyObject *self, PyObject *other)
{
	return __aop__(self, other, bwand, "&");
}

/**
 * (static) Numeric XOR protocol. Calls self->__aop__(other, ...) with
 * approperiate arguments for bitwise XOR.
 */
PyObject *EnumeratedConstantObject::__xor__(PyObject *self, PyObject *other)
{
	return __aop__(self, other, bwxor, "^");
}

/**
 * (static) Numeric OR protocol. Calls self->__aop__(other, ...) with
 * approperiate arguments for bitwise OR.
 */
PyObject *EnumeratedConstantObject::__or__(PyObject *self, PyObject *other)
{
	return __aop__(self, other, bwor, "|");
}

/**
 *
 */
PyObject *EnumeratedConstantObject::__aop__(PyObject *self, PyObject *other,
		                                    long (*arith)(long,long), 
											const char *opname)
{
	int self_value, other_value;
	bool typeerror = false;

	// Translate 'self' to an integer value
	if (EnumeratedConstantObject_Check(self)) {
		self_value =
			((EnumeratedConstantObject*)self)->m_underlying->getValue();
	}
	else if (PyInt_Check(self)) {
		self_value = PyInt_AsLong(self);
	}
	else {
		typeerror = true;
	}

	// Translate 'other' to an integer value
	if (EnumeratedConstantObject_Check(other)) {
		other_value =
			((EnumeratedConstantObject*)other)->m_underlying->getValue();
	}
	else if (PyInt_Check(other)) {
		other_value = PyInt_AsLong(other);
	}
	else {
		typeerror = true;
	}

	if (typeerror) {
		PyErr_Format(PyExc_TypeError, 
				     "unsupported operands types for '%s'", opname);
		return NULL;
	}
	else {
		return PyInt_FromLong(arith(self_value, other_value));
	}
}

/**
 * Returns the enumerated constant held in this object.
 */
Handle<EnumeratedConstant> EnumeratedConstantObject::getUnderlying() const
{
	return m_underlying;
}


/**
 * ConversionHookObject constructor.
 */
ConversionHookObject::ConversionHookObject(Handle<Class> underlying,
										   Kind kind)
	: m_underlying(underlying), m_kind(kind)
{
	PyObject_Init(this, &ConversionHookTypeObject);
}

ConversionHookObject::~ConversionHookObject()
{
}

/**
 * (static) Set-subscript protocol function - invokes __setsubscript__.
 */
int ConversionHookObject::__setsubscript__(PyObject *self, 
										   PyObject *sub, PyObject *val)
{
	return ((ConversionHookObject*)self)->__setsubscript__(sub, val);
}

/**
 * (static) Deallocator - called by Python when the object is destroyed.
 */
void ConversionHookObject::__dealloc__(PyObject *self)
{
	delete (ConversionHookObject*)self;
}

/**
 * Implements the SETSUBSCRIPT protocol by registering the requested
 * conversion from the given object to the destination.
 * Can optionally get a tuple containing the weight of the conversion
 * as well as the function that performs the conversion itself.
 */
int ConversionHookObject::__setsubscript__(PyObject *sub, PyObject *val)
{
	// Check if the given value is a tuple, and if so parse it
	PyObject *callable  = val;
	PyObject *pyweight  = NULL;
	PyObject *pyweigher = NULL;
	int       weight    = 1;
	int       promotion = 0;

	if (PyTuple_Check(val)) {
		callable  = PyTuple_GetItem(val, 0);
		if (PyTuple_Size(val) > 1) {
			pyweight  = PyTuple_GetItem(val, 1);
			weight    = 0;
			promotion = (int)PyInt_AsLong(pyweight);
			if (PyTuple_Size(val) > 2) {
				pyweigher = PyTuple_GetItem(val, 2);
			}
		}
	}

	// Create a handler
	Handle<PyCallableWithInstance> handler = pycallableFactory(callable);
	if (PyErr_Occurred()) {
		return -1;
	}

	// Assign to applicable conversion slot
	Handle<Conversion> conv(pyweigher 
						? new PythonConversionWithWeigher(handler, pyweigher)
						: new PythonConversion(handler));
	try {
		conv->setSourceType(FrontendsFramework::activeFrontend()
						       ->detectType(sub));
		conv->setTargetType((m_kind == VOLATILE) ? m_underlying->getOutArg() 
							                     : m_underlying->getRefArg());
		Conversion::Weight conv_weight(0, promotion, 0, weight);
		conv->setWeight(conv_weight);
		ConversionTableSingleton::getInstance()
			->registerConversion(conv);
		return 0;
	}
	catch (UnsupportedInterfaceException& uie) {
		PyErr_Format(PyExc_TypeError, "type '%s' is unfamiliar",
					 sub->ob_type->tp_name);
		return -1;
	}
}


/**
 * Service routine, checks whether a Python object is a FunctionObject
 *
 * @param object a Python object to check
 */
bool FunctionObject_Check(PyObject *object)
{
	return PyType_IsSubtype((PyTypeObject*)PyObject_Type(object),
							&FunctionTypeObject);
}

/**
 * Service routine, checks whether a Python object is a ClassObject
 *
 * @param object a Python object to check
 */
bool ClassObject_Check(PyObject *object)
{
	return PyType_IsSubtype((PyTypeObject*)PyObject_Type(object),
							ClassTypeObject);
}

/**
 * Returns the inners' dictionary of a given class object.
 * Only call this method if ClassObject_Check(object) is true.
 *
 * @return a borrowed reference to a Python dictionary
 */
PyObject *ClassObject_GetDict(PyObject *object)
{
	return ((ClassObject*)object)->getDict();
}

/**
 * Service routine, checks whether a Python object is an InstanceObject.
 *
 * @param object a Python object to check
 */
bool InstanceObject_Check(PyObject *object)
{
	return ClassObject_Check(PyObject_Type(object));
}

/**
 * Service routine, checks whether a Python object is an AddressObject.
 *
 * @param object a Python object to check
 */
bool AddressObject_Check(PyObject *object)
{
	return (PyObject_Type(object) == (PyObject*)&AddressTypeObject);
}

/**
 * Service routine, checks whether a Python object is an 
 *EnumeratedConstantObject.
 *
 * @param object a Python object to check
 */
bool EnumeratedConstantObject_Check(PyObject *object)
{
	return PyObject_TypeCheck(PyObject_Type(object),
							  &EnumeratedTypeTypeObject);
}

/**
 * Frees a Pascal string, used as a deallocator when creating a PyCObject
 * holding a Pascal string. This function is called automatically by Python
 * when the object is destroyed.
 *
 * @param ppascal an opaque pointer to the Pascal string object, received from
 * the Python runtime.
 */
void PyPascalString_deallocator(void *ppascal)
{
	free(ppascal);
}

/**
 * Service routine, checks whether a Python object holds a Pascal string.
 *
 * @param object a Python object to check
 */
bool PyPascalString_Check(PyObject *object)
{
	return (PyCObject_Check(object));
}


PyTypeObject *makeMetaclassType(const char *name, PyTypeObject *base)
{
  PyObject *bases = PyTuple_New(1);
  PyTuple_SET_ITEM(bases, 0, (PyObject*)base);
  Py_XINCREF(&PyType_Type);

  PyObject *args = PyTuple_New(3);
  PyTuple_SET_ITEM(args, 0, PyString_FromString(name));
  PyTuple_SET_ITEM(args, 1, bases);
  PyTuple_SET_ITEM(args, 2, PyDict_New());
  PyObject *type = PyObject_Call((PyObject*)&PyType_Type, args, 0);
  Py_XDECREF(args);

  return (PyTypeObject*)type;
}


void initObjects()
{
	// Initialize ClassTypeObject
	ClassTypeObject = makeMetaclassType("Robin::Python::ClassObject",
										&PyType_Type);
	ClassTypeObject->tp_dealloc = ClassObject::__dealloc__;
	ClassTypeObject->tp_getattr = ClassObject::__getattr__;
	ClassTypeObject->tp_setattr = ClassObject::__setattr__;
	ClassTypeObject->tp_getattro = 0;
	ClassTypeObject->tp_setattro = 0;
	ClassTypeObject->tp_repr = ClassObject::__repr__;
	ClassTypeObject->tp_call = ClassObject::__call__;
	ClassTypeObject->tp_new = HybridObject::__new_hybrid__;

	// Initialize HybridTypeObject
	HybridTypeObject = makeMetaclassType("Robin::Python::HybridObject",
										 ClassTypeObject);
	HybridTypeObject->tp_dealloc = ClassObject::__dealloc__;
    HybridTypeObject->tp_getattr = ClassObject::__getattr__;
    HybridTypeObject->tp_setattr = ClassObject::__setattr__;
    HybridTypeObject->tp_getattro = 0;
    HybridTypeObject->tp_setattro = 0;
}


} // end of namespace Python

} // end of namespace Robin
