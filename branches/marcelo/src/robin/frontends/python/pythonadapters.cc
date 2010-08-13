// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/python/pythonadapters.cc
 *
 * @par TITLE
 * Python Frontend Adapters Implementation
 *
 * @par PACKAGE
 * Robin
 */

#include <Python.h>

#include <sstream>

#include "pythonadapters.h"
#include "pythonobjects.h"

#include <robin/debug/assert.h>


// Robin includes
#include <robin/reflection/class.h>
#include <robin/reflection/instance.h>
#include <robin/reflection/address.h>
#include <robin/reflection/enumeratedtype.h>
#include <robin/reflection/pascalstring.h>
#include <robin/reflection/conversion.h>
#include <robin/reflection/fundamental_conversions.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/conversiontable.h>

#include <pattern/map.h>
#include <pattern/handle.h>


namespace Robin {

namespace Python {

Handle<RobinType> ArgumentPythonTuple
	= RobinType::create_new(TYPE_CATEGORY_EXTENDED, TYPE_EXTENDED_VOID, "python tuple",RobinType::constReferenceKind);
Handle<RobinType> ArgumentPythonLong
	= RobinType::create_new(TYPE_CATEGORY_EXTENDED, TYPE_EXTENDED_VOID, "python long",RobinType::constReferenceKind);


bool PyBoolTraits::as(PyObject *pyobj)
{
	if (pyobj == Py_True)
		return true;
	else if (pyobj == Py_False)
		return false;
	else
		return PyInt_AsLong(pyobj);
}

PyObject *PyBoolTraits::from(bool val)
{
	PyObject *result = val ? Py_True : Py_False;
	Py_XINCREF(result);
	return result;
}


/**
 * InstanceAdapter constructor.
 */
InstanceAdapter::InstanceAdapter(ClassObject *classobj, bool owned)
	: m_class(classobj), m_owned(owned)
{
}

/**
 * Puts pointer to instance on arguments buffer.
 */
void InstanceAdapter::put(ArgumentsBuffer& args, scripting_element value)
{
	const InstanceObject *instobj = reinterpret_cast<InstanceObject*>(value);
	const void *instptr = instobj->getUnderlying()->getObject();
	args.pushPointer(instptr);

	if (m_owned) instobj->getUnderlying()->disown();
}

/**
 * Retrieves returned pointer and creates an InstanceObject.
 */
scripting_element InstanceAdapter::get(basic_block data)
{
	void *pinstance = LowLevel::reinterpret_lowlevel<void*>(data);
	if (pinstance) {
		Handle<Instance> hinstance(new Instance(m_class->getUnderlying(), 
												pinstance, m_owned));
		PyObject *object = new InstanceObject(m_class, hinstance);
		return (scripting_element)object;
	}
	else {
		Py_INCREF(Py_None);
		return (scripting_element)Py_None;
	}
}


/**
 * AddressAdapter constructor.
 *
 * @param domain the type of the pointed datum (i.e. int).
 */
AddressAdapter::AddressAdapter(Handle<RobinType> domain)
	: m_domain(domain)
{ 
}

void AddressAdapter::put(ArgumentsBuffer& argsbuf, scripting_element value)
{
	AddressObject *obj = (AddressObject*)value;
	argsbuf.push(obj->getUnderlying()->asPointer());
}

scripting_element AddressAdapter::get(basic_block data)
{
	Handle<Address> address(new Address(m_domain, 
										reinterpret_cast<void*>(data)));
	PyObject *obj = new AddressObject(address);
									  
	return (scripting_element)obj;
}


/**
 * EnumeratedAdapter constructor.
 */
EnumeratedAdapter::EnumeratedAdapter(EnumeratedTypeObject *pyenumtype)
	: m_type(pyenumtype)
{
}

EnumeratedAdapter::~EnumeratedAdapter()
{
}

void EnumeratedAdapter::put(ArgumentsBuffer& argsbuf, scripting_element value)
{
	EnumeratedConstantObject *obj = (EnumeratedConstantObject*)value;
	argsbuf.push(obj->getUnderlying()->getValue());
}

scripting_element EnumeratedAdapter::get(basic_block data)
{
	long enumvalue = LowLevel::reinterpret_lowlevel<long>(data);
	Handle<EnumeratedConstant> constant
		(new EnumeratedConstant(m_type->getUnderlying(), enumvalue));
	PyObject *object = new EnumeratedConstantObject(m_type, constant);
	return (scripting_element)object;
}


/**
 * Pascal strings are passed as arguments using CObjects.
 */
void PascalStringAdapter::put(ArgumentsBuffer& args, scripting_element value)
{
	PyObject *pystr = (PyObject *)value;
	PascalString pascal;
	pascal.size  = PyString_Size(pystr);
	pascal.chars = PyString_AsString(pystr);
	args.push((long)pascal.size);
	args.push((void*)pascal.chars);
}

/**
 * Constructs a Python string from a Pascal string.
 */
scripting_element PascalStringAdapter::get(basic_block data)
{
	PascalString *pascal = reinterpret_cast<PascalString*>(data);
	PyObject *python = PyString_FromStringAndSize(pascal->chars, pascal->size);
#ifndef _WIN32
	// TODO: should wrap 'delete' in the library side.
	// @@@ win32: the loader does not allow an object allocated in one DLL
	// to be deleted in another. Currently this results in a memory leak
	// in the win32 runtime.
	delete pascal;
#endif
	return python;
}

/**
 * Puts the pointer to a Python object on the arguments buffer.
 */
void PyObjectAdapter::put(ArgumentsBuffer& args, scripting_element value)
{
	PyObject *pyelement = (PyObject *)value;
	// - CObjects are used to encapsulate any Python element
	if (PyCObject_Check(pyelement))
		pyelement = (PyObject*)PyCObject_AsVoidPtr(pyelement);
	args.pushPointer(pyelement);
}

/**
 * Returns the scripting element returned from a function as a PyObject.
 */
scripting_element PyObjectAdapter::get(basic_block data)
{
	return reinterpret_cast<scripting_element>(data);
}


} // end of namespace Python

} // end of namespace Robin
