// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/python/module.cc
 *
 * @par TITLE
 * Python Frontend Implementation
 *
 * @par PACKAGE
 * Robin
 */

#include <Python.h>

#include <string>
#include <stdexcept>
#include "pythonadapters.h"
#include "pythonfrontend.h"
#include "pythonobjects.h"
#include "inheritance.h"
#include "wrappedrobintype.h"
#include "pythonerror.h"
#include "types/listrobintype.h"
#include "types/dictrobintype.h"

#include <robin/reflection/instance.h>
#include <robin/registration/mechanism.h>
#include <robin/frontends/framework.h>
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/library.h>
#include <robin/reflection/robintype.h>
#include <robin/debug/trace.h>

#include <robin/reflection/address.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/const.h>
#include <robin/reflection/pointer.h>

void nothing() {
	std::cout << " inside" <<std::endl;
}

namespace Robin {

namespace Python {

class PythonFrontend::Module {
public:
	static Handle<PythonFrontend> fe;

	static PyObject *py_loadLibrary(PyObject *self, PyObject *args)
	{
		const char *libname, *libfile = 0;
		if (!PyArg_ParseTuple(args, "s|s:loadLibrary", &libname, &libfile))
			return NULL;
		try {
			if (libfile == 0) { libfile = libname; libname = 0; }
			// Load and register library
			RegistrationMechanism *mech =
				RegistrationMechanismSingleton::getInstance();
			Handle<Library> library = 
				(libname == 0) ? mech->import(libfile) 
							   : mech->import(libfile, libname);
			// Expose library
			fe->exposeLibrary(*library);
			// done
			Py_INCREF(Py_None);
			return Py_None;
		}
		catch (const DynamicLibraryOpenException& dle) {
			PyErr_SetString(PyExc_ImportError, dle.dlerror_at.c_str());
			return NULL;
		}
		catch (std::exception& e) {
			PyErr_SetString(PyExc_ImportError, e.what());
			return NULL;
		}
	}

	static PyObject * py_debugOn(PyObject *self, PyObject *args)
	{
		dbg::trace.enable();
		Py_INCREF(Py_None);
		return Py_None;
	}
	static PyObject * py_debugRegistrationOn(PyObject *self, PyObject *args)
	{
		dbg::traceRegistration.enable();
		Py_INCREF(Py_None);
		return Py_None;
	}

	static PyObject * py_debugClassRegistrationOn(PyObject *self, PyObject *args)
	{
		dbg::traceClassRegistration.enable();
		Py_INCREF(Py_None);
		return Py_None;
	}

	static PyObject * py_classByName(PyObject *self, PyObject *args)
	{
		const char *classname;

		if (!PyArg_ParseTuple(args, "s", &classname))
			return NULL;

		ClassObject *klass = fe->getClassObject(classname);
		if (klass) {
			Py_XINCREF(klass);
			return (PyObject*)klass;
		}
		else {
			PyErr_Format(PyExc_KeyError, "no such Robin class: '%s'",
						 classname);
			return NULL;
		}
	}

	static PyObject * py_setTemplateObject(PyObject *self, PyObject *args)
	{
		const char *templatename;
		PyObject *pytemplate;

		if (!PyArg_ParseTuple(args, "sO", &templatename, &pytemplate))
			return NULL;

		fe->setTemplateObject(templatename, (TemplateObject*)pytemplate);
		if (PyErr_Occurred()) return NULL;

		Py_INCREF(Py_None);
		return Py_None;
	}


	static PyObject * py_weighConversion(PyObject *self, PyObject *args)
	{
		PyObject *sourcetype, *source;
		PyObject *desttype  , *dest;
		PyObject *sourcevalue = NULL;
		bool source_ismasq=false;

		if (!PyArg_ParseTuple(args, "OO", &sourcetype, &desttype))
			return NULL;

		Handle<RobinType> toadst;
		if (PyType_Check(desttype)) {
				// masquerade will be able to be removed
			    // when python will frontend fully support
				// detectType from type
				dest	= masquerade((PyTypeObject*)desttype);
				try {
					toadst =
						fe->detectType_mostSpecific((scripting_element)dest);
				} catch (std::exception& e) {
					PyErr_SetString(PyExc_RuntimeError, e.what());
					return NULL;
				}
				unmasquerade(dest);
		}else if(PyObject_TypeCheck(desttype, &WrappedRobinTypeTypeObject))
		{
			toadst = ((WrappedRobinType*)desttype)->m_underlying;
		} else {
			PyErr_SetString(PyExc_TypeError, "expected 2nd argument to be a type");
			return NULL;
		}

		// Compute conversion
		Conversion::Weight w;

		// this could be a value
		if(!PyType_Check(sourcetype)) {
			sourcevalue = sourcetype;
		}

		// if we have a source value, detect the type from it
		// instead of the real 'source'
		if(sourcevalue != NULL) {
			source = sourcevalue;
			source_ismasq = false;
		} else {
			source = masquerade((PyTypeObject*)sourcetype);
			source_ismasq = true;
		}

		try {
			Handle<RobinType>
				toasrc = fe->detectType_mostSpecific((scripting_element)source);

			Handle<ConversionRoute> convRoute =
				ConversionTableSingleton::getInstance()->
				bestSingleRoute(*toasrc, *toadst);
			w = convRoute->totalWeight();
		}
		catch(const NoApplicableConversionException& ) {
			w = Conversion::Weight::INFINITE;
		}
		catch (const std::exception& ) {
			w = Conversion::Weight::INFINITE;
		}

		if (source_ismasq) unmasquerade(source);


		PyObject* wtuple =
			Py_BuildValue("(iiii)", 
					w.getEpsilon(), w.getPromotion(), 
					w.getUpcast(), w.getUserDefined());
		return wtuple;
	}

	/**
	 * A horrible patch for creating an object of the given type without
	 * invoking any constructors.
	 */
	static PyObject *masquerade(PyTypeObject *type)
	{
		PyObject *agent;

		if (type == &PyString_Type) {
			agent = PyString_FromString("agent");
		}
		else {
			agent = PyMem_New(PyObject, 1);
			PyObject_Init(agent, type);
		}
		return agent;
	}

	/**
	 * Used to release masquerade() objects.
	 */
	static void unmasquerade(PyObject *agent)
	{
		if (agent->ob_type == &PyString_Type) {
			Py_XDECREF(agent);
		}
		else {
			PyMem_Del(agent);
		}
	}

	static void py_xdecrefv(void *v) { Py_XDECREF((PyObject*)v); }

	/**
	 * Returns a PyCObject which encapsulates a Python object.
	 */
	static PyObject *py_obscure(PyObject *self, PyObject *args)
	{
		PyObject *pyobj;

		if (!PyArg_ParseTuple(args, "O:obscure", &pyobj)) 
			return NULL;

		Py_XINCREF(pyobj);
		return PyCObject_FromVoidPtr(pyobj, &py_xdecrefv);
	}

	/**
	 * Steals ownership over the C++ instance in an InstanceObject.
	 */
	static PyObject *py_disown(PyObject *self, PyObject *args)
	{
		PyObject *pyobj;

		if (!PyArg_ParseTuple(args, "O:disown", &pyobj)) 
			return NULL;

		if (InstanceObject_Check(pyobj)) {
			InstanceObject *instance = (InstanceObject*)pyobj;
			instance->getUnderlying()->disown();
		}

		Py_XINCREF(pyobj);
		return pyobj;
	}

	/**
	 * Steals ownership over the C++ instance in an InstanceObject.
	 */
	static PyObject *py_own(PyObject *self, PyObject *args)
	{
		PyObject *pyobj;

		if (!PyArg_ParseTuple(args, "O:own", &pyobj)) 
			return NULL;

		if (InstanceObject_Check(pyobj)) {
			InstanceObject *instance = (InstanceObject*)pyobj;
			instance->getUnderlying()->own();
		}

		Py_XINCREF(pyobj);
		return pyobj;
	}

	/**
	 * Add a custom translator.
	 */
	static PyObject *py_familiarize(PyObject *self, PyObject *args)
	{
		PyObject *pyobj;

		if (!PyArg_ParseTuple(args, "O:familiarize", &pyobj)) 
			return NULL;

		if (!PyType_Check(pyobj)) {
			PyErr_SetString(PyExc_TypeError, "expected: type");
			return NULL;
		}

		Handle<UserDefinedTranslator> translate
			(new ByTypeTranslator((PyTypeObject*)pyobj));
		fe->addUserDefinedType(translate);
		
		Py_XINCREF(Py_None);
		return Py_None;
	}

	/**
	 * Either create a new NULL pointer of a particular type
	 * or create a pointer to pointer.
	 * (temporary function for investigating
	 * the new pointer type mechanism and address objects)
	 */
	static PyObject *py_pointer(PyObject *self, PyObject *args)
	{
		PyObject *pyobj;
		Handle<Robin::RobinType> rtype;

		if (!PyArg_ParseTuple(args, "O:pointer", &pyobj)) 
			return NULL;

		try {
			if(ClassObject_Check(pyobj)) {
				/*
				 * py_pointer is only for testing purposes.
				 * currently one level pointers (A *) are not
				 * represented by address objects but only by
				 * their pointer type accessible from Class::getPtrType()
				 * So using py_pointer in a type which is a class
				 * will provide now a two level pointer (A **)
				 */
				rtype = ((ClassObject*)pyobj)->getUnderlying()->getPtrType();
				int *ptr = new int(0);
				Handle<Robin::Address> haddress(new Robin::Address(rtype,
																   ptr));
				return new Robin::Python::AddressObject(haddress);
			} else if (PyType_Check(pyobj)) {
				/*
				 * for the rest of the types py_pointer will
				 * provide a one level pointer (A *)
				 */
				rtype = fe->detectType((PyTypeObject*)pyobj);

				int *ptr = new int(0);
				Handle<Robin::Address> haddress(new Robin::Address(rtype, 
																   ptr));
				return new Robin::Python::AddressObject(haddress);
			}
			else if (Robin::Python::AddressObject_Check(pyobj)) {
				/*
				 * if we got a pointer it will increase by one the level of the pointer.
				 */
				return new Robin::Python::AddressObject(((Robin::Python::AddressObject*)pyobj)->getUnderlying()->reference());
			}
			else {
				PyErr_SetString(PyExc_TypeError, "expected type or address");
				return NULL;
			}
		}
		catch (std::exception& e) {
			PyErr_SetString(PyExc_RuntimeError, (char*)e.what());
			return NULL;
		}
	}

	/**
	 * Dereference a pointer. (temporary function for investigating
	 * the new pointer type mechanism and address objects)
	 */
	static PyObject *py_dereference(PyObject *self, PyObject *args)
	{
		PyObject *pyobj;
		long subscript = 0;

		if (!PyArg_ParseTuple(args, "O|i:dereference", &pyobj, &subscript)) 
			return NULL;

		return (PyObject*)((Robin::Python::AddressObject*)pyobj)->getUnderlying()->dereference(subscript);
	}

	static PyObject *py_preResolve(PyObject *self, PyObject *args)
	{
		PyObject *raw_func;

		if (!PyArg_ParseTuple(args, "O", &raw_func))
		{
			return NULL;
		}
		if(!FunctionObject_Check(raw_func))
		{
			PyErr_SetString(PyExc_TypeError, "preResolve needs to get one robin function or method");
			return NULL;
		}

		FunctionObject *func = (FunctionObject*) raw_func;
		PyReferenceSteal<FunctionObject> preResolver = func->preResolver();

		return preResolver.release();
	}

	static PyObject * py_weighCall(PyObject *self, PyObject *args, PyObject *kw)
	{
		    // Construct arguments
			Py_ssize_t nargs = PyTuple_Size(args) -1;
			if(nargs < 0 ) {
				PyErr_SetString(PyExc_TypeError,"weightCall needs to get at least one parameter (the function to weight)");
			}


			PyObject *raw_func = PyTuple_GetItem(args,0);
			if(!FunctionObject_Check(raw_func))
			{
				PyErr_SetString(PyExc_TypeError, "weightCall needs to get one robin function or method in the first parameter");
				return NULL;
			}
			FunctionObject *func = (FunctionObject*) raw_func;


			Handle<ActualArgumentList> pass_args(new ActualArgumentList(nargs));
			for (Py_ssize_t argindex = 0; argindex < nargs; ++argindex)
			{
				(*pass_args)[argindex] = PyTuple_GetItem(args, argindex+1);
			}

		    // XXX: initialize from kw
		    KeywordArgumentMap kwargs;

		    if(kw != NULL && kw != Py_None) {
				assert_true(PyDict_Check(kw));
		        PyObject *key, *value;
		        Py_ssize_t pos = 0;
		        while(PyDict_Next(kw, &pos, &key, &value)) {
		            std::string kwmap_key(PyString_AsString(key));
		            kwargs[kwmap_key] = value;
		        }
		    }

		    Handle<WeightList> weightList = func->weight(pass_args,kwargs);
		    if(!weightList) {
		    	return NULL;
		    }
		    // Similarly to py_weighConversion() we return tuples instead of Weight objects
		    // might need to change this in the future.

		    PyObject* pythonWeightList = PyList_New(weightList->size());
		    WeightList::const_iterator it = weightList->begin();
		    WeightList::const_iterator end = weightList->end();
		    for(Py_ssize_t i=0;it!=end;it++,i++)
		    {
		    	PyObject *tuple = Py_BuildValue("(iiii)",
		    								it->getEpsilon(), it->getPromotion(),
		    								it->getUpcast(), it->getUserDefined());
		    	if(!tuple) {
		    		Py_XDECREF(pythonWeightList);
		    		return NULL;
		    	}
		    	PyList_SET_ITEM(pythonWeightList , i , tuple);
		    }
			return pythonWeightList;
	}



	static PyObject *py_searchOrCreateListType(PyObject *self, PyObject *args)
	{
		PyObject *elementType;
		if (!PyArg_ParseTuple(args, "O:searchOrCreateListType", &elementType))
		{
				return NULL;
		}
		if(!PyObject_TypeCheck(elementType, &WrappedRobinTypeTypeObject))
		{
			PyErr_SetString(PyExc_TypeError,"searchOrCreateListType expected a RobinType as its parameter ");
			return NULL;
		}

		try {
			WrappedRobinType *elementType_cast =(WrappedRobinType *)elementType;
			Handle<RobinType> listType = ListRobinType::listForSpecificElements(elementType_cast->m_underlying);
			return WrappedRobinType::construct(listType).release();
		} catch(const std::exception &err) {
			PyErr_SetString(PyExc_RuntimeError,err.what());
			return NULL;
		}
	}

	static PyObject *py_searchOrCreateDictType(PyObject *self, PyObject *args)
	{
		PyObject *keyType;
		PyObject *valueType;
		if (!PyArg_ParseTuple(args, "OO:searchOrCreateDictType", &keyType,&valueType))
		{
				return NULL;
		}
		if(!PyObject_TypeCheck(keyType, &WrappedRobinTypeTypeObject))
		{
			PyErr_SetString(PyExc_TypeError,"searchOrCreateDictType expected a RobinType as its first parameter ");
			return NULL;
		}
		if(!PyObject_TypeCheck(valueType, &WrappedRobinTypeTypeObject))
		{
			PyErr_SetString(PyExc_TypeError,"searchOrCreateDictType expected a RobinType as its second parameter ");
			return NULL;
		}

		try {
			WrappedRobinType *keyType_cast =(WrappedRobinType *)keyType;
			WrappedRobinType *valueType_cast =(WrappedRobinType *)valueType;
			Handle<RobinType> dictType = DictRobinType::dictForSpecificKeyAndValues(keyType_cast->m_underlying,valueType_cast->m_underlying);
			return WrappedRobinType::construct(dictType).release();
		} catch(const std::exception &err) {
			PyErr_SetString(PyExc_RuntimeError,err.what());
			return NULL;
		}
	}

	static PyObject * py_searchOrCreateConstType(PyObject *self, PyObject *args)
	{
		PyObject *wrappedRobinType;
		if (!PyArg_ParseTuple(args, "O:py_searchOrCreateConstType", &wrappedRobinType))
		{
				return NULL;
		}
		if(!PyObject_TypeCheck(wrappedRobinType, &WrappedRobinTypeTypeObject))
		{
			PyErr_SetString(PyExc_TypeError,"py_searchOrCreateConstType: robinType parameter should be of type RobinType");
			return NULL;
		}
		WrappedRobinType *wrappedRobinType_cast =(WrappedRobinType *)wrappedRobinType;
		Handle<RobinType> robinType = wrappedRobinType_cast->m_underlying;
		if(!robinType) {
			PyErr_SetString(PyExc_TypeError,"Internal logic error with parameter robinType");
			return NULL;
		}

		if(robinType->isConstant()) {
			PyErr_SetString(PyExc_TypeError,"robinType should not be a constant type itself.");
			return NULL;
		}

		try {
			Handle<RobinType> constType = ConstType::searchOrCreate(*robinType);
			PyReferenceSteal<WrappedRobinType> constTypeWrapped = WrappedRobinType::construct(constType);
			return constTypeWrapped.release();
		} catch(const std::exception &err) {
			PyErr_SetString(PyExc_RuntimeError,err.what());
			return NULL;
		}
	}

	static PyObject * py_nothing(PyObject *self, PyObject *args, PyObject *kw)
	{
		nothing();
		Py_XINCREF(Py_None);
		return Py_None;
	}

	static PyMethodDef methods[];
};


PyMethodDef PythonFrontend::Module::methods[] = {
	{ "loadLibrary", &py_loadLibrary, METH_VARARGS, 
	  "loadLibrary([libname], libfile)\nloads a library "
	  "using Robin's registration mechanism." },
	{ "classByName", &py_classByName, METH_VARARGS, 
	  "classByName(classname)\nFetches a registered class from Robin's "
	  "internal registry." },
	{ "declareTemplate", &py_setTemplateObject, METH_VARARGS,
	  "declareTemplate(string templatename, object pytemplate)\n"
	  "Sets a user-defined template object." },
	{ "weighConversion", &py_weighConversion, METH_VARARGS,
	  "weighConversion(type source, type destination)\n"
	  "Calculates the weight of the conversion between the source type and the"
	  "destination type" },
	{ "obscure", &py_obscure, METH_VARARGS,
	  "obscure(object)\n"
	  "Causes the given object to be interpreted as a 'scripting_element',\n"
	  "ignoring its actual concrete type." },
	{ "own", &py_own, METH_VARARGS,
	  "own(object)\n"
	  "Seizes ownership of a given instance object, such that when the\n"
	  "reference is destroyed, the object itself is also deleted." },
	{ "disown", &py_disown, METH_VARARGS,
	  "disown(object)\n"
	  "Ceases ownership of a given instance object, such that when the\n"
	  "reference is destroyed, the object itself is not deleted." },
	{ "familiarize", &py_familiarize, METH_VARARGS,
	  "familiarize(type)\n"
	  "Makes the given Python type known to Robin, allowing user to define\n"
	  "custom conversions to/from it." },
	{ "debugOn", &py_debugOn, METH_VARARGS, 
	  "debugOn()\nTurns on Robin's internal debug flag." },
	{ "debugRegistrationOn", &py_debugRegistrationOn, METH_VARARGS,
	  "debugRegistrationOn()\nTurns on Robin's internal debug flag for events related to registration of any entity." },
	{ "debugClassRegistrationOn", &py_debugClassRegistrationOn, METH_VARARGS,
	  "debugClassRegistrationOn()\nTurns on Robin's internal debug flag for events related to registration of classes." },
	{ "pointer", &py_pointer, METH_VARARGS, "" },
	{ "dereference", &py_dereference, METH_VARARGS, "" },
	{ "preResolve", &py_preResolve, METH_VARARGS, "Recieves a robin method or a function by parameter, and return an equivalent callable which works faster.\n"
												  "The method used is to store the invocation resolution after the first time so it doesnt need to be reolved again. "
												  "The invocation resolution means which signature the call has (which version of the function to call) and which "
												  " conversion to perform on the parameters." },
    { "weighCall", (PyCFunction)&py_weighCall, METH_VARARGS|METH_KEYWORDS,
    	"it weighs the cost of the type conversions of each parameter in a function/method call.\n"
    	"The first parameter is the function or bounded method, the rest are the parameters for the method"
    },
    { "nothing", (PyCFunction)&py_nothing, METH_VARARGS|METH_KEYWORDS,
    	"It returns None. Can be used to set a breakpoint in gdb."
    },
    {"searchOrCreateListType",(PyCFunction)&py_searchOrCreateListType,METH_VARARGS,
    	"searchOrCreateListType(elementRobinType) --> RobinType\n"
    	"\n"
    	"It searches for the list type of elementRobinType and returns it.\n"
    	"Possibly it will have to create it.\n"
		"parameter elementRobinType: it is a RobinType of the elements of the list."
    },
    {"searchOrCreateDictType",(PyCFunction)&py_searchOrCreateDictType,METH_VARARGS,
    	"searchOrCreateDictType(keyRobinType,valueRobinType) --> RobinType\n"
    	"\n"
    	"It searches for the list type of the given keys and values types and returns it.\n"
    	"Possibly it will have to create it.\n"
		"parameter keyType: it is a RobinType of the keys of the dict.\n"
		"parameter valueType: it is a RobinType of the values of the dict.\n"

    },
    {"searchOrCreateConstType",(PyCFunction)&py_searchOrCreateConstType,METH_VARARGS,
    	"searchOrCreateConstType(elementRobinType) --> RobinType\n"
    	"\n"
    	"It searches for the const type of elementRobinType and returns it.\n"
    	"Possibly it will have to create it."
    },
   { 0 }
};

Handle<PythonFrontend> PythonFrontend::Module::fe;


PyObject *pydouble;
PyObject *pychar;
PyObject *pylong_long;
PyObject *pyunsigned_long;
PyObject *c_int;
PyObject *c_long;
PyObject *pyunsigned_int;
PyObject *pyunsigned_long_long;
PyObject *pyunsigned_char;
PyObject *pysigned_char;
PyObject *c_short;
PyObject *c_ushort;
PyObject *c_float;



} // end of namespace Robin::Python

} // end of namespace Robin

using namespace Robin;
using namespace Robin::Python;


/*
 * It wraps a RobinType, it stores in a python module and it stores
 * a pointer to it in a variable.
 * @param module where to insert the wrapped object
 * @param name the name of the type being wrapped
 * @param robinType the type to wrap
 * @param storage a temporary pointer to cache the
 *
 *
 */
static  void addWrappedType(
		PyObject *module,
		char *name,
		Handle<RobinType> &robintype,
		PyObject *&storage)
{
	PyReferenceSteal<WrappedRobinType>
		wrapped = WrappedRobinType::construct(robintype);
	storage = wrapped.release();
	PyModule_AddObject(module, name, storage);
}


void initlibrobin_pyfe();


#ifndef _ROBIN_SPEC
#error The macro _ROBIN_SPEC has to be defined in the compiler command line
#error It defines the suffix added to all the library names to specify the current version
#endif

//ROBININIT generates the initializing function of the module
#define _ROBININIT(NAME,SPEC) init##NAME##SPEC
#define ROBININIT(NAME,SPEC) _ROBININIT(NAME,SPEC)

//ROBINMODULESTRING generates the name of the module quoted
#define _ROBINSTRING(PARAM) #PARAM
#define ROBINSTRING(PARAM) _ROBINSTRING(PARAM)
#define ROBINMODULESTRING(NAME,SPEC) ROBINSTRING(NAME) ROBINSTRING(SPEC)


//initializing function
PyMODINIT_FUNC
ROBININIT(librobin_pyfe,_ROBIN_SPEC) () {
	initlibrobin_pyfe();
};

void initlibrobin_pyfe()
{
	using Robin::Python::PythonFrontend;

	// Activate frontend
	PythonFrontend::Module::fe = Handle<PythonFrontend>(new PythonFrontend);
	Robin::FrontendsFramework::selectFrontend(
		static_hcast<Robin::Frontend>(PythonFrontend::Module::fe));

	Robin::Python::initObjects();

	// Register Python module
	PyObject *module =
		Py_InitModule4(ROBINMODULESTRING(librobin_pyfe,_ROBIN_SPEC), PythonFrontend::Module::methods,
					   "Robin Wrapper Module", NULL, PYTHON_API_VERSION);

	if (PyType_Ready(&Robin::Python::WrappedRobinTypeTypeObject) < 0)
	        return;


	// Add some stuff to the module
	Py_XINCREF(&Robin::Python::FunctionTypeObject);
	Py_XINCREF(Robin::Python::ClassTypeObject);
	Py_XINCREF(Robin::Python::EnumeratedTypeTypeObject);


	PyModule_AddObject(module, "FunctionType", 
					   (PyObject*)&Robin::Python::FunctionTypeObject);
	PyModule_AddObject(module, "ClassType", 
					   (PyObject*)Robin::Python::ClassTypeObject);
	PyModule_AddObject(module, "EnumeratedTypeType", 
					   (PyObject*)Robin::Python::EnumeratedTypeTypeObject);
	addWrappedType(module, "double", ArgumentDouble, pydouble);
	addWrappedType(module, "longlong", ArgumentLongLong, pylong_long);
	addWrappedType(module, "c_int", ArgumentInt,c_int);
	addWrappedType(module, "uint", ArgumentUInt,pyunsigned_int);
	addWrappedType(module, "c_long", ArgumentLong, c_long);
	addWrappedType(module, "ulong",ArgumentULong,pyunsigned_long);
	addWrappedType(module, "ulonglong", ArgumentULongLong, pyunsigned_long_long);
	addWrappedType(module, "char", ArgumentChar, pychar);
	addWrappedType(module, "uchar", ArgumentUChar, pyunsigned_char);
	addWrappedType(module, "schar", ArgumentSChar, pysigned_char);
	addWrappedType(module, "short", ArgumentShort, c_short);
	addWrappedType(module, "ushort", ArgumentShort, c_ushort);
	addWrappedType(module, "c_float", ArgumentFloat, c_float);

	Py_XINCREF(&Robin::Python::WrappedRobinTypeTypeObject);
	PyModule_AddObject(module,"RobinType",(PyObject*)&Robin::Python::WrappedRobinTypeTypeObject);

#define _QUOTE(X) (#X)
#define QUOTE(X) (_QUOTE(X))
	PyModule_AddObject(module, "VERSION", 
					   PyString_FromString(QUOTE(_VERSION)));

	Py_XINCREF(Py_True);
	PyModule_AddObject(module, "true",  Py_True);
	Py_XINCREF(Py_False);
	PyModule_AddObject(module, "false", Py_False);

}
