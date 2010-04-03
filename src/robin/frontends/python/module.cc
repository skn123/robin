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
#include <cstdlib>

#include "pythonfrontend.h"
#include "pythonobjects.h"
#include "inheritance.h"

#include <robin/reflection/instance.h>
#include <robin/registration/mechanism.h>
#include <robin/frontends/framework.h>
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/library.h>
#include <robin/debug/trace.h>

#include <robin/reflection/address.h>
#include <robin/reflection/intrinsic_type_arguments.h>


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

		if (!PyType_Check(desttype)) {
			PyErr_SetString(PyExc_TypeError, "expected 2nd argument to be a type");
			return NULL;
		}

		// Compute conversion
		Conversion::Weight w;

		if (PyCObject_Check(desttype)) {
			w = Conversion::Weight::INFINITE;
		}
		else {

			// this could be a value
			if(!PyType_Check(sourcetype)) {
				sourcevalue = sourcetype;
			}

			dest	= masquerade((PyTypeObject*)desttype);

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
				Handle<TypeOfArgument>
					toasrc = fe->detectType((scripting_element)source),
					toadst = fe->detectType((scripting_element)dest);
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
			unmasquerade(dest);
		}

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
	 * TEMPORARY create a pointer.
	 */
	static PyObject *py_pointer(PyObject *self, PyObject *args)
	{
		PyObject *pyobj;
		Handle<Robin::TypeOfArgument> rtype;

		if (!PyArg_ParseTuple(args, "O:pointer", &pyobj))
			return NULL;

		try {
			if (PyType_Check(pyobj)) {
				rtype = fe->detectType((PyTypeObject*)pyobj);

				int *ptr = new int(0);
				Handle<Robin::Address> haddress(new Robin::Address(rtype,
																   ptr));
				return new Robin::Python::AddressObject(haddress);
			}
			else if (Robin::Python::AddressObject_Check(pyobj)) {
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
	 * TEMPORARY dereference a pointer.
	 */
	static PyObject *py_dereference(PyObject *self, PyObject *args)
	{
		PyObject *pyobj;
		long subscript = 0;

		if (!PyArg_ParseTuple(args, "O|i:dereference", &pyobj, &subscript))
			return NULL;

		return (PyObject*)((Robin::Python::AddressObject*)pyobj)->getUnderlying()->dereference(subscript);
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
	{ "pointer", &py_pointer, METH_VARARGS, "" },
	{ "dereference", &py_dereference, METH_VARARGS, "" },
	{ 0 }
};

Handle<PythonFrontend> PythonFrontend::Module::fe;

PyObject *pydouble;
PyObject *pychar;
PyObject *pylong_long;
PyObject *pyunsigned_long;
PyObject *pyunsigned_int;
PyObject *pyunsigned_long_long;
PyObject *pyunsigned_char;
PyObject *pysigned_char;

} // end of namespace Robin::Python

} // end of namespace Robin

extern "C"
#ifdef _WIN32
__declspec(dllexport)
#endif
void initrobin()
{
	using Robin::Python::PythonFrontend;

	// Activate frontend
	PythonFrontend::Module::fe = Handle<PythonFrontend>(new PythonFrontend);
	Robin::FrontendsFramework::selectFrontend(
		static_hcast<Robin::Frontend>(PythonFrontend::Module::fe));

	Robin::Python::initObjects();

	// Register Python module
	PyObject *module =
		Py_InitModule4("robin", PythonFrontend::Module::methods,
					   "Robin Wrapper Module", NULL, PYTHON_API_VERSION);

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
	PyModule_AddObject(module, "double", Robin::Python::pydouble =
					   PyCObject_FromVoidPtr((void*)"double", 0));
	PyModule_AddObject(module, "char", Robin::Python::pychar =
					   PyCObject_FromVoidPtr((void*)"char", 0));
	PyModule_AddObject(module, "longlong", Robin::Python::pylong_long =
					   PyCObject_FromVoidPtr((void*)"long long", 0));
	PyModule_AddObject(module, "uint", Robin::Python::pyunsigned_int =
					   PyCObject_FromVoidPtr((void*)"unsigned int", 0));
	PyModule_AddObject(module, "ulong", Robin::Python::pyunsigned_long =
					   PyCObject_FromVoidPtr((void*)"unsigned long", 0));
	PyModule_AddObject(module, "ulonglong",
					   Robin::Python::pyunsigned_long_long =
					   PyCObject_FromVoidPtr((void*)"unsigned long long", 0));
	PyModule_AddObject(module, "uchar", Robin::Python::pyunsigned_char =
					   PyCObject_FromVoidPtr((void*)"unsigned char", 0));
	PyModule_AddObject(module, "schar", Robin::Python::pysigned_char =
					   PyCObject_FromVoidPtr((void*)"signed char", 0));

#define _QUOTE(X) (#X)
#define QUOTE(X) (_QUOTE(X))
	PyModule_AddObject(module, "VERSION",
					   PyString_FromString(QUOTE(_VERSION)));

#ifndef _MSC_VER
	PyModule_AddObject(module, "true",  Py_True);
	PyModule_AddObject(module, "false", Py_False);
#endif
}
