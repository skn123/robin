// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/python/enhancements.h
 *
 * @par TITLE
 * Python Frontend - Enhancements
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_PYTHON_ENHANCEMENTS_H
#define ROBIN_PYTHON_ENHANCEMENTS_H

// STL includes
#include <string>

// Python includes
#include <Python.h>

// Pattern includes
#include <pattern/handle.h>

// Robin includes
#include <robin/reflection/conversion.h>


namespace Robin {

namespace Python {

class ClassObject;
class InstanceObject;

/**
 * A functor callable with a Python instance and Python arguments.
 */
class PyCallableWithInstance
{
public:
	virtual PyObject *callUpon(InstanceObject *self, PyObject *args) = 0;
};

/**
 * A simple event dispatcher with predefined slots.
 */
class EnhancementsPack 
{
public:
	typedef Handle<PyCallableWithInstance> Handler;

	enum SlotID {
		PRINT,
		STRING,
		REPR,
		HASH,
		CALL,
		/* Sequence slots */
		LENGTH,
		GETITEM,
		SETITEM,
		DELITEM,
		GETSLICE,
		SETSLICE,
		DELSLICE,
		/* Number slots */
		ADD,            R_ADD,
		SUBTRACT,       R_SUBTRACT,
		MULTIPLY,       R_MULTIPLY,
		DIVIDE,         R_DIVIDE,
		MODULO,         R_MODULO,
		POWER,          R_POWER,
		BW_AND,         R_BW_AND,
		BW_OR,          R_BW_OR,
		BW_XOR,         R_BW_XOR,
		BW_NOT,
		LSHIFT,         R_LSHIFT,
		RSHIFT,         R_RSHIFT,
		TO_OCT,
		TO_HEX,
		TO_INT,
		TO_FLOAT,
		/* Mapping slots */
		GETSUBSCRIPT,
		SETSUBSCRIPT,
		DELSUBSCRIPT,
		MAPSIZE,
		/* Rich comparison slots (new in Python 2.2) */
		EQUALS,         R_EQUALS,
		NEQUAL,         R_NEQUAL,
		LESS_THAN,      R_LESS_THAN,
		GREATER_THAN,   R_GREATER_THAN,
		LESS_OR_EQ,     R_LESS_OR_EQ,
		GREATER_OR_EQ,  R_GREATER_OR_EQ,

		NSLOTS
	};

	EnhancementsPack();

	void setSlot(SlotID slot, Handler action);
	bool supports(SlotID slot) const;
	PyObject *trigger(SlotID slot, InstanceObject *self, PyObject *args);

	static EnhancementsPack::SlotID slotByName(const std::string& name);

	/**
	 * Thrown when trying to access a non-existent slot.
	 */
	class NoSuchSlotException : public std::exception
	{
	public:
		const char *what() const throw() { return "no such slot"; }
	};

private:
	Handler m_slots[NSLOTS];
};

/**
 */
class Protocol
{
public:
	typedef EnhancementsPack::SlotID SlotID;
	typedef EnhancementsPack::Handler Handler;

	virtual void deploy(ClassObject *type, 
						EnhancementsPack::SlotID slot,
						EnhancementsPack::Handler handler) = 0;
	
	static Protocol& deployerBySlot(EnhancementsPack::SlotID slot);
	static Protocol& deployerBySlotName(const std::string& slotname);

	static void autoEnhance(ClassObject *type);
};

/**
 * Implements a simple functor by invoking an instance method.
 */
class InstanceMethodFunctor : public PyCallableWithInstance
{
public:
	InstanceMethodFunctor(const std::string& methodname);
	virtual PyObject *callUpon(InstanceObject *self, PyObject *args);	

private:
	std::string m_methodname;
};

/**
 * Implements a simple functor by invoking a Python function or callable
 * object.
 */
class PyObjectNativeFunctor : public PyCallableWithInstance
{
public:
	PyObjectNativeFunctor(PyObject *pycallable);
	~PyObjectNativeFunctor();
	virtual PyObject *callUpon(InstanceObject *self, PyObject *args);

private:
	PyObject *m_pycallable;
};

/**
 * A conversion implemented in Python.
 */
class PythonConversion : public Conversion
{
public:
	PythonConversion(Handle<PyCallableWithInstance> functor);

	scripting_element apply(scripting_element value) const;

	void reportConversionError() const;

private:
	Handle<PyCallableWithInstance> m_functor;
};

/**
 * A conversion implemented in Python with an insight weigher implemented
 * in Python as well.
 */
class PythonConversionWithWeigher : public PythonConversion
{
public:
	PythonConversionWithWeigher(Handle<PyCallableWithInstance> functor,
								PyObject *weigher);
	~PythonConversionWithWeigher();

	using PythonConversion::weight;
	virtual Weight weight(Insight insight) const;

private:
	PyObject *m_weigher;
};


} // end of namespace Python

} // end of namespace Robin

#endif
