// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/python/pythonobjects.h
 *
 * @par TITLE
 * Python Frontend Programmer's Facade
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_PYTHON_FACADE_H
#define ROBIN_PYTHON_FACADE_H

#include <Python.h>
#include <robin/reflection/instance.h>


namespace Robin {


class UserDefinedTranslator;


namespace Python {


/**
 * Provides a neat programming interface into the Robin Python Frontend.
 * Using this facade, persons writing their own (manually coded) extension
 * modules for Python can interact with objects generated and wrapped by
 * Robin's runtime.
 *
 * @par operations
 * Basically, the facade allows the following activities:
 * <ul>
 *  <li>Check whether a Python object is an instance of a specific Robin class
 *      [check(const char *, PyObject *)].</li>
 *  <li>Extract a reference to an actual C++ object wrapped by the object
 *      [asObject(const char *, PyObject *)].</li>
 *  <li>Create a new wrapped object from a plain C++ pointer
 *      [fromObject(const char *, 
 */
class Facade {
public:
	typedef const char *classdescriptor;

	/**
	 * @name Types
	 */
	//@{
	static PyTypeObject  *type(classdescriptor classname);
	static bool           check(classdescriptor classname, PyObject *pyobj);

	static void           userDefined(Handle<UserDefinedTranslator> translate);
	//@}

	/**
	 * @name C++/Python Conversion
	 */
	//@{
	static PyObject      *fromObject(classdescriptor classname, void *objptr,
									   bool owner = true);
	template <class T>
	static T             *asObject(classdescriptor classname, PyObject *pyobj);
	template <class T>
	static inline T      *asObject_trustme(PyObject *pyobj);
	//@}

	/**
	 * @name Advanced Reflection
	 */
	//@{
	static Handle<Class>    asClass(const std::string& classname);
	static Handle<Instance> asInstance(PyObject *pyobj);
	//@}

protected:
	static void *object2pointer(PyObject *pyobj);
};


} // end of namespace Robin::Python

} // end of namespace Robin


#include "facade.inl"

#endif
