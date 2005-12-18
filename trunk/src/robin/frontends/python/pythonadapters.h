// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/python/pythonadapters.h
 *
 * @par TITLE
 * Python Frontend Adapters Implementation
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_PYTHON_ADAPTERS_H
#define ROBIN_PYTHON_ADAPTERS_H

#include <robin/frontends/adapter.h>
#include <robin/reflection/low_level.h>
#include <robin/reflection/typeofargument.h>
#include <Python.h>


namespace Robin {


namespace Python {


class ClassObject;
class EnumeratedTypeObject;


//@{

class PyIntTraits { 
public:
	static long as(PyObject *pyobj) { return PyInt_AsLong(pyobj); }
	static PyObject *from(long val) { return PyInt_FromLong(val); }
};

class PyLongTraits {
public:
	static long long as(PyObject *pyobj)     { return PyLong_AsLongLong(pyobj); }
	static PyObject *from(long long val)     { return PyLong_FromLongLong(val); }
	static long long *asref(PyObject *pyobj) { return new long long(as(pyobj)); }
};

class PyBoolTraits {
public:
	static bool as(PyObject *pyobj);
	static PyObject *from(bool val);
};

class Py1CharStringTraits {
public:
	static char as(PyObject *pyobj) { return PyString_AsString(pyobj)[0]; }
	static PyObject *from(char val) 
	                        { return PyString_FromStringAndSize(&val, 1); }
};

class PyStringTraits {
public:
	static const char *as(PyObject *pyobj) { return PyString_AsString(pyobj); }
	static PyObject *from(const char *s)   { if (!s) s = "";
	                                         return PyString_FromString(s); }
};

class PyFloatTraits {
public:
	static double as(PyObject *pyobj)      { return PyFloat_AsDouble(pyobj); }
	static double *asref(PyObject *pyobj)  { return new double(as(pyobj));   }
	static PyObject *from(double val)      { return PyFloat_FromDouble(val); }
};	

class PyFloatSmallTraits {
public:
	static float as(PyObject *pyobj) { return float(PyFloat_AsDouble(pyobj)); }
	static PyObject *from(double val){ return PyFloat_FromDouble(val); }
};	

//@}


/**
 * @class SmallPrimitivePythonAdapter
 * @nosubgrouping
 */
template < class CType, class Traits >
class SmallPrimitivePythonAdapter : public Adapter
{
public:
	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value) {
		argsbuf.push(Traits::as((PyObject*)value));
	}
	virtual scripting_element get(basic_block data) {
		return (scripting_element)Traits::from(LowLevel::reinterpret_lowlevel<CType>(data));
	}
};

/**
 * @class SmallPrimitivePythonAdapter
 * @nosubgrouping
 */
template < class CType, class Traits >
class SmallPrimitiveReinterpretPythonAdapter : public Adapter
{
public:
	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value) {
		argsbuf.push(Traits::as((PyObject*)value));
	}
	virtual scripting_element get(basic_block data) {
		union u { basic_block bb; CType ct; } u; u.bb = data;
		return (scripting_element)Traits::from(u.ct);
	}
};

/**
 * @class SmallPrimitiveDerefPythonAdapter
 * @nosubgrouping
 */
template < class CType, class Traits >
class SmallPrimitiveDerefPythonAdapter : public Adapter
{
public:
	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value) {
		argsbuf.push(Traits::as((PyObject*)value));
	}
	virtual scripting_element get(basic_block data) {
		return (scripting_element)Traits
			::from(*reinterpret_cast<CType*&>(data));
	}
};

/**
 * @class AllocatedPrimitivePythonAdapter
 * @nosubgrouping
 */
template < class CType, class Traits >
class AllocatedPrimitivePythonAdapter : public Adapter
{
public:
	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value) {
		argsbuf.push(Traits::asref((PyObject*)value));
	}
	virtual scripting_element get(basic_block data) {
		CType *dtp = (CType*)data;
		scripting_element pye = (scripting_element)Traits::from(*dtp);
#ifndef _WIN32
		// @@@ win32: the loader does not allow an object allocated in one DLL
		// to be deleted in another. Currently this results in a memory leak
		// in the win32 runtime.
		delete dtp;
#endif
		return pye;
	}
};

/**
 * @class AllocatedBigPrimitivePythonAdapter
 * @nosubgrouping
 */
template < class CType, class Traits >
class AllocatedBigPrimitivePythonAdapter : public Adapter
{
	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value) {
		argsbuf.push(Traits::as((PyObject*)value));
	}
	virtual scripting_element get(basic_block data) {
		CType *dtp = (CType*)data;
		scripting_element pye = (scripting_element)Traits::from(*dtp);
#ifndef _WIN32
		// @@@ win32: the loader does not allow an object allocated in one DLL
		// to be deleted in another. Currently this results in a memory leak
		// in the win32 runtime.
		delete dtp;
#endif
		return pye;
	}
};

/**
 * Converts a Pascal-style string to a Python string (keeping original size
 * and data and maintaining embedded nulls if they occur).
 */
class PascalStringAdapter : public Adapter
{
public:
	virtual ~PascalStringAdapter() { }

	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
	virtual scripting_element get(basic_block data);
};

/**
 * Converts between a C++ pointer and a Robin::Python::InstanceObject.
 * @nosubgrouping
 */
class InstanceAdapter : public Adapter
{
public:
	InstanceAdapter(ClassObject *classobj, bool owned);
	virtual ~InstanceAdapter() { }

	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
	virtual scripting_element get(basic_block data);

private:
	ClassObject *m_class;
	bool m_owned;
};


/**
 * Converts between a C++ enumerated value and a
 * Robin::Python::EnumeratedConstantObject.
 * @nosubgrouping
 */
class EnumeratedAdapter : public Adapter
{
public:
	EnumeratedAdapter(EnumeratedTypeObject *pyenumtype);
	virtual ~EnumeratedAdapter();

	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
	virtual scripting_element get(basic_block data);

private:
	EnumeratedTypeObject *m_type;
};


class PyObjectAdapter : public Adapter
{
public:
	virtual ~PyObjectAdapter() { }

	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
	virtual scripting_element get(basic_block data);	
};

// Python-specific argument types
extern Handle<TypeOfArgument> ArgumentPythonList;
extern Handle<TypeOfArgument> ArgumentPythonTuple;
extern Handle<TypeOfArgument> ArgumentPythonDict;
extern Handle<TypeOfArgument> ArgumentPythonLong;


} // end of namespace Python

} // end of namespace Robin


#endif
