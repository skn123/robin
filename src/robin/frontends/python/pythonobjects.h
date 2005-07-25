// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/python/pythonobjects.h
 *
 * @par TITLE
 * Python Frontend Objects
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_PYTHON_OBJECTS_H
#define ROBIN_PYTHON_OBJECTS_H

#include <limits>

// Python includes
#include <Python.h>

// Pattern includes
#include <pattern/handle.h>

// Robin includes
#include <robin/reflection/callable.h>
#include "enhancements.h"


namespace Robin {

class Class;
class Instance;
class EnumeratedConstant;

namespace Python {

class InstanceObject;

/**
 * Exposes a routine to the Python interpreter.
 * Supports the __call__ protocol, and delegates invocation to a 
 * <classref>Callable</classref> object.
 */
class FunctionObject : public PyObject
{
public:
	FunctionObject(Handle<Callable> underlying);
	FunctionObject(Handle<CallableWithInstance> underlying,
				   InstanceObject *self);
	~FunctionObject();

	void setName(const std::string& name);
	void setSelf(InstanceObject *self);
	void inModule(PyObject *module);

	PyObject *__call__(PyObject *args, PyObject *kw);
	PyObject *__getattr__(const char *name);
	PyObject *__repr__();

	static PyObject *__call__(PyObject *self, PyObject *args, PyObject *kw);
	static PyObject *__getattr__(PyObject *self, char *name);
	static PyObject *__repr__(PyObject *self);
	static void __dealloc__(PyObject *self);

private:
	Handle<Callable>             m_underlying;
	Handle<CallableWithInstance> m_underlying_thiscall;
	InstanceObject              *m_self;
	PyObject                    *m_in_module;

	std::string                  m_name;
};

/**
 * Represents a class in the internal reflection.
 * <p>Supported protocols:</p>
 * <ul><li>__call__ - forwards call to class' constructors.</li>
 * </ul>
 */
class ClassObject : public PyTypeObject
{
public:
	ClassObject(Handle<Class> underlying);
	~ClassObject();

	PyObject *__call__(PyObject *args, PyObject *kw);
	PyObject *__getattr__(const char *attrname);
	int       __setattr__(char *name, PyObject *val);
	PyObject *__repr__();

	static PyObject *__new__    (PyTypeObject *self, 
								 PyObject *args, PyObject *kw);
	static PyObject *__call__   (PyObject *self, PyObject *args, PyObject *kw);
	static PyObject *__getattr__(PyObject *self, char *attrname);
	static int       __setattr__(PyObject *self, char *name, PyObject *val);
	static PyObject *__repr__   (PyObject *self);
	static void      __dealloc__(PyObject *self);

	/**
	 * @name Access
	 */
	//@{
	void            inModule(PyObject *module);

	Handle<Class>   getUnderlying() const;
	PyObject       *getContainingModule() const;
	PyObject       *getDict() const;

	Handle<CallableWithInstance> findInstanceMethod(const char *name,
													const char **internal_name)
		const;
	//@}

	/**
	 * @name Enhancements
	 */
	//@{
	EnhancementsPack& getEnhancements();
	const EnhancementsPack& getEnhancements() const;
	//@}

private:
	Handle<Class>     m_underlying;
	PyObject         *m_in_module;    //!< Python module containing this class
	PyObject         *m_inners;       //!< A Python dictionary of st. members
	EnhancementsPack  m_enhance;

	// - optimization
	struct MethodDef {
		const char *name;
		const char *internal_name;
		Handle<CallableWithInstance> meth;
	};
	mutable MethodDef *m_x_methods;

	void x_optimizeMethodTable() const;
	inline Handle<CallableWithInstance> x_findMethod(const char *name,
													 const char **internal)
		const;
	void x_releaseMethodTable();
};

/**
 * Represents a class instance.
 */
class InstanceObject : public PyObject
{
public:
	InstanceObject(ClassObject *classobj, Handle<Instance> underlying);
	~InstanceObject();

	PyObject *__getattr__(const char *attrname);
	int       __setattr__(char *name, PyObject *val);
	PyObject *__repr__();

	static PyObject *__getattr__(PyObject *self, char *attrname);
	static int       __setattr__(PyObject *self, char *name, PyObject *val);
	static PyObject *__repr__(PyObject *self);
	static void      __dealloc__(PyObject *self);

	/**
	 * @name Access
	 */
	//@{
	Handle<Instance> getUnderlying() const;
	PyObject *getBoundMethodOrDataMember(const char *name);

	void keepAlive(PyObject *owner);
	//@}

private:
	Handle<Instance> m_underlying;
	PyObject        *m_ownership_keep;

public:
	PyObject        *m_ob_weak;
};

/**
 */
class EnumeratedConstantObject : public PyObject
{
public:
	EnumeratedConstantObject(Handle<EnumeratedConstant> underlying);
	~EnumeratedConstantObject();

	PyObject *__repr__();
	bool      __richcmp__(EnumeratedConstantObject *other, int opid);
	PyObject *__int__() const;

	static
	PyObject *__aop__(PyObject *self, PyObject *other,
			          long (*arith)(long,long),
			          const char *opname);

	static PyObject *__repr__(PyObject *self);
	static PyObject *__richcmp__(PyObject *self, PyObject *other, int opid);
	static PyObject *__int__(PyObject *self);
	static PyObject *__and__(PyObject *self, PyObject *other);
	static PyObject *__xor__(PyObject *self, PyObject *other);
	static PyObject *__or__ (PyObject *self, PyObject *other);
	static void      __dealloc__(PyObject *self);

	/**
	 * @name Access
	 */
	//@{
	Handle<EnumeratedConstant> getUnderlying() const;
	//@}

private:
	Handle<EnumeratedConstant> m_underlying;
};

/**
 * A dummy object which allows user to assign conversions from/to a robin
 * class type, by implementing the SETSUBSCRIPT protocol.
 */
class ConversionHookObject : public PyObject
{
public:
	enum Kind { NORMAL, VOLATILE };

	ConversionHookObject(Handle<Class> underlying, Kind = NORMAL);
	~ConversionHookObject();

	int __setsubscript__(PyObject *sub, PyObject *val);

	static int  __setsubscript__(PyObject *self, PyObject *sub, PyObject *val);
	static void __dealloc__(PyObject *self);

private:
	Handle<Class> m_underlying;
	Kind m_kind;
};

// ClassObject, InstanceObject services
bool ClassObject_Check(PyObject *object);
PyObject *ClassObject_GetDict(PyObject *object);
bool InstanceObject_Check(PyObject *object);

// Pascal string services
void PyPascalString_deallocator(void *);
bool PyPascalString_Check(PyObject *object);



extern PyTypeObject FunctionTypeObject;
extern PyTypeObject ClassTypeObject;
extern PyTypeObject EnumeratedConstantTypeObject;


} // end of namespace Python

} // end of namespace Robin


#endif
