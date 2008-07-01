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

#include "facade.h"
#include "pythonfrontend.h"
#include "pythonobjects.h"
#include "../framework.h"


namespace Robin {

namespace Python {


/**
 * Returns a Robin class object for the given class name.
 *
 * @param classname a fully-qualified C++ class name
 * @return A handle to a Robin::Class instance, or a null handle if no
 * registered class matches the requested name
 *
 * @par note
 * This function is provided for advanced use cases of Robin. If you use it,
 * you'd better know what you're doing.
 */
Handle<Class> Facade::asClass(const std::string& classname)
{
	PythonFrontend *pyfe =
		dynamic_cast<PythonFrontend*>(FrontendsFramework::activeFrontend());
	assert(pyfe != NULL);

	ClassObject *pyclass = pyfe->getClassObject(classname);
	if (pyclass) {
		return pyclass->getUnderlying();
	}
	else {
		return Handle<Class>();
	}
}

/**
 * Returns an instance object held in a PyObject.
 * If the object is not a Robin instance object, a Python error is set and
 * a null handle is returned.
 *
 * @par note
 * This function is provided for advanced use cases of Robin. If you use it,
 * you'd better know what you're doing.
 */
Handle<Instance> Facade::asInstance(PyObject *pyobj)
{
	PyObject     *pytype = PyObject_Type(pyobj);

	if (ClassObject_Check(pytype)) {
		return ((InstanceObject*)pyobj)->getUnderlying();
	}
	else {
		PyErr_SetString(PyExc_TypeError, "expected: Robin instance");
		return Handle<Instance>();
	}
}

/**
 * Returns the type object associated with the given class of Robin's.
 * If no class by that name exists in Robin's reflection, a Python error is
 * set and <b>NULL</b> is returned.
 */
PyTypeObject *Facade::type(classdescriptor classname)
{
	// Fetch the currently active PythonFrontend
	Frontend *fe = FrontendsFramework::activeFrontend();
	assert(fe != NULL);
	PythonFrontend *pyfe = dynamic_cast<PythonFrontend *>(fe);
	assert(pyfe != NULL);

	// Get the class from the frontend's internal maps
	ClassObject *clas = pyfe->getClassObject(classname);
	if (clas == NULL) {
		PyObject *py_errorstr = PyString_FromFormat("no such class - '%s'",
													classname);
		PyErr_SetObject(PyExc_KeyError, py_errorstr);
	}

	Py_XINCREF(clas);
	return clas;
}

/**
 * Checks whether the given 'pyobj' is a Robin instance object of the class
 * referred to by 'classname'.
 */
bool Facade::check(classdescriptor classname, PyObject *pyobj)
{
	PyObject     *pytype = PyObject_Type(pyobj);

	if (ClassObject_Check(pytype)) {
		Handle<Class> klass = ((InstanceObject*)pyobj)
			->getUnderlying()->getClass();
		return (klass->name() == classname);
	}
	else
		return false;
}

/**
 * Wraps an existing C++ instance object as a Robin::Python::InstanceObject.
 * The class is given by <b>literal name</b>, the object is passed by address,
 * and the boolean 'owner' flag determines whether the newly created instance
 * object receives ownership over the C++ instance.
 */
PyObject *Facade::fromObject(classdescriptor classname, void *objptr,
							 bool owner)
{
	// Find my class
	ClassObject *classobj = (ClassObject*)type(classname);
	if (classobj == NULL) return NULL;

	// Create an instance
	Handle<Instance> instance(new Instance(classobj->getUnderlying(), objptr,
										   owner));
	return new InstanceObject(classobj, instance);
}

/**
 * Registers a user-defined Python type.
 *
 * @param translate a UserDefinedTranslator instance with an implementation for
 * the new type.
 */
void Facade::userDefined(Handle<UserDefinedTranslator> translate)
{
	PythonFrontend *pyfe = 
		dynamic_cast<PythonFrontend*>(FrontendsFramework::activeFrontend());
	assert(pyfe != NULL);

	pyfe->addUserDefinedType(translate);
}

/**
 * (internal) Takes the pointer held in a Robin instance object, without
 * affecting the wrapped object or the wrapper, and returns the address.
 *
 * @par note
 * This routine does no checking, so the programmer is responsible for making
 * sure the object really is a Robin::Python::InstanceObject, or unexpected
 * behaviors may be introduced.
 */
void *Facade::object2pointer(PyObject *pyobj)
{
	InstanceObject *pyinst = static_cast<InstanceObject*>(pyobj);
	return (pyinst->getUnderlying()->getObject());
}


} // end of namespace Robin::Python

} // end of namespace Robin
