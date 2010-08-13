// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * Starting from Oct/2009 we want to start using these automatic handlers to
 * properly hold python references.
 */

#ifndef ROBIN_FRONTENDS_PYTHON_PYHANDLE_H
#define ROBIN_FRONTENDS_PYTHON_PYHANDLE_H

#include "robinpyobject.h"
#include <Python.h>
#include <stdexcept>
#include "pylegalconversions.h"

#ifdef ROBIN_PYOBJECTHANDLE_DEBUG
#	include "../../../pattern/handle.h"
#endif


namespace Robin {
namespace Python {

/*
 * Just so you know, these are the three types of reference
 * handles defined in this file. Each one behaves a little different.
 */
template <typename T> class PyReferenceBorrow;
template <typename T> class PyReferenceCreate;
template <typename T> class PyReferenceSteal;

/**
 * Common abstract base class for other handlers.
 * Each handle differs from the rest on whatever they increase their refcount
 * when recieving the PyObject and if they decrease their refcount in their
 * destructor
 * Read the rest of the class for more information
 * Class T has to inherit from type PyObject
 */
template<typename T>
class PyObjectHandle
{

	friend class PyReferenceBorrow<T>;
	friend class PyReferenceCreate<T>;
	friend class PyReferenceSteal<T>;

public:
    bool operator == (const PyObjectHandle<T>& other) const
   	{
    	return (m_instance == other.m_instance);
   	}
    bool operator != (const PyObjectHandle<T>& other) const
   	{
    	return (m_instance != other.m_instance);
   	}



    /**
     * @name Pointer-like Interface
     */
    //@{
    inline T * operator -> ()
    {
    	checkParent();
    	return (T*)m_instance;
    }

    inline const T * operator -> () const
    {
    	checkParent();
      	return (const T*)m_instance;
    }

    inline T & operator * ()  {
    	checkParent();
    	return *(T*)m_instance;
    }

    inline const T & operator * () const {
    	checkParent();
    	return *(const T*)m_instance;
    }

    inline operator bool() const
    {
    	checkParent();
    	return m_instance != 0;
    }

    /**
     * Returns a pointer to the instance.
     * Notice that a pointer obtained this way has semantics of a borrowed reference.
     */
    T*pointer ()
    {
    	return (T*)m_instance;
    }


    /**
     * Releases responsibilities of disposing its reference. The pointer is returned,
     * which enables the caller to do at some moment do Py_XDEVREF to the object.
     */
    T*release ()
    {
#        ifdef ROBIN_PYOBJECTHANDLE_DEBUG
    	*m_parentAlive = false;
#		endif
    	T* instance = (T*)m_instance;
    	m_instance = 0;
    	return instance;
    }

    /**
     * Returns a pointer to the instance.
     * Notice that a pointer obtained this way has semantics of a borrowed reference.
     */
    const T* pointer () const
    {
    	return (const T*)m_instance;
    }

	//@}

protected:
    inline void incRef() const
	{
    	checkParent();
    	Py_XINCREF(const_cast<PyObject *>(m_instance));
	}


    /**
     * The intance pointed to is held as a PyObject to make sure
     * that the type pointed to is capable of working with the
     * python reference counting system.
     */
    PyObject * m_instance;

    /**
     * Objects from this class should never be created/deleted
     * Only derived classes
     */
    inline ~PyObjectHandle() {

    }


private:

    /**
     *    It only has an effect on PyReferenceBorrow.
     *    It checks that the reference we borrowed from has not been released.
     *    If the flag ROBIN_PYOBJECTHANDLE_DEBUG is not set, it does not do anything.
     */
	inline void checkParent() const
	{
#        ifdef ROBIN_PYOBJECTHANDLE_DEBUG
			if(! *m_parentAlive )
			{
				//the error can only happen in PyReferenceBorrow objects
				throw std::logic_error("PyReferenceBorrow used when its parent has already been released ");
			}
#        endif
	}
protected:

#   ifdef ROBIN_PYOBJECTHANDLE_DEBUG
	/**
	 * It is a boolean used to debug.
	 * It is true for any PythonOwnHandle but it can be false for PyReferenceBorrow.
	 * A PyReferenceBorrow holds a pointer to the same m_parentAlive as its parent and
	 * knows when the parent reference was released because the value will be set to false.
	 */
	Handle<bool> m_parentAlive;
#        endif
};

/**
 * Handles deriving from this class have the responsability to
 * free their references (they own a reference).
 * It is still abstract.
 */
template<typename T>
class PythonOwnHandle : public PyObjectHandle<T>
{
public:

	/*
	 * The class PythonOwnHandle_ref is used as a tool to be able to return
	 * PythonOwnHandles from functions (and also to be able to recieve them
	 * as parameters by value).
	 * The technique was copied from the design of std::auto_ptr.
	 * The problem is the following:
	 *    C++ has a security mechanism that does not enable to call functions of the
	 *    like 'foo(OBJ &obj);' with a temporary obj as argument (temporary is an object
	 *    created by the compiler implicit,  when returning objects or passing by value
	 *    (if it needs to do implicit type convertions)
	 *
	 *    Returning an object from a functions implicitly call the copy constructor, that
	 *    means that it calls OBJ(OBJ) (a constructor of OBJ with an argument of type OBJ).
	 *
	 *    The only possibility for regular classes is to call OBJ(const OBJ &) because calling
	 *    OBJ(OBJ &) collides with the security mechanism previously defined.
	 *
	 *  The solution adopted is the following:
	 *    We create our own reference ('OBJ_ref' instead of 'OBJ &'). The OBJ_ref is passed to
	 *    the constructor of OBJ by value. When the constructor tries to call to OBJ(OBJ) the actual
	 *    call done is OBJ(OBJ_ref(OBJ)).
	 *    What we are actually saying to C++ is that we do not need the defence against passing
	 *    a temporary object (the defence does not apply to pointers and references in first
	 *    place).
	 *
	 *  A word about efficiency:
	 *    The object behaves exactly as a reference.
	 *    But also notice that the common compilers know how to do
	 *    RVO(return value optimization) and NRVO (named return value optimization) which
	 *    means that the copy constructor for OBJ will never be called (still it has to be
	 *    defined).
	 *
	 *  Notice:
	 *    C++ automatically creates a copy constructor  OBJ(const OBJ&) if there are no constructors
	 *    defined which get type OBJ, in order to cancel the automatic construction we need to
	 *    hand-write OBJ(OBJ &). That constructor will never be called automatically when returning
	 *    or passing OBJ by value, so its only purpose is to cancell OBJ(const OBJ &).
	 *    In spite of that it is important to define it properly because the programmer can call it
	 *    explicitly.
	 *
	 */
	class PythonOwnHandle_ref {
	private:
		PythonOwnHandle & m_handle;
		friend class PythonOwnHandle<T>;
		friend class PyReferenceSteal<T>;
		explicit PythonOwnHandle_ref(PythonOwnHandle<T> &ref)
			: m_handle(ref)
		{

		}
	};

	inline operator PythonOwnHandle_ref()
	{
		return PythonOwnHandle_ref(*this);
	}

protected:
	inline PythonOwnHandle(T *instance)
	{
		pointedChanged(instance);
	}


	inline void pointedChanged(T * instance)
	{
		PyObjectHandle<T>::m_instance = convertToPyObject(instance);
#        ifdef ROBIN_PYOBJECTHANDLE_DEBUG
			PyObjectHandle<T>::m_parentAlive = Handle<bool>(new bool(true));
#		endif
	}


	/**
	 * Objects of this class should never be deleted/constructed directly,
	 * only from derived classes.
	 */
    inline ~PythonOwnHandle() {
    	decRef();
    }

    inline void decRef() const
    {
#        ifdef ROBIN_PYOBJECTHANDLE_DEBUG
    	*PyObjectHandle<T>::m_parentAlive = false;
#		endif
    	Py_XDECREF(const_cast<PyObject *>(PyObjectHandle<T>::m_instance));
    }

};

/**
 * @class PyReferenceSteal
 *
 * A handle pointing to python objects. I tholds a python reference formally which means it
 * needs to dec the reference count in its destructor.
 * It is used for two things:
 *  - To pass to a function a reference which will be stolen (the function will take care of
 *    decreasing the refcount, the caller needs to increase the refcount if it wants to
 *    store another copy of the reference).
 *  - To return a reference from a function ( which means the caller is now respoinsible
 *  for decreasing the ref count).
 *
 * PyReferenceSteal steals the reference, which means:
 *     - It will never increase the reference count
 *     - At some point it will decrease the reference count, possibly deleting the object.
 */
template <typename T>
class PyReferenceSteal : public PythonOwnHandle<T>
{

public:
	using PythonOwnHandle<T>::decRef;
	using PythonOwnHandle<T>::m_instance;
	using PythonOwnHandle<T>::pointedChanged;


	/**
	 * Only calls the destructor from the base class
	 */
	inline ~PyReferenceSteal() {

	}

	inline PyReferenceSteal()
		: PythonOwnHandle<T>(0)
	{

	}

	/**
	 * Creates a new object from a PyObject*.
	 * The old reference to the object is 'stolen' which means that
	 * the PyReferenceSteal will be the responsible for freeing it.
	 */
    inline explicit PyReferenceSteal(T *ptr)
		: PythonOwnHandle<T>(ptr)
    {

    }

    /**
     * Steals a reference from a PythonOwnHandle
     */
    inline PyReferenceSteal(typename PythonOwnHandle<T>::PythonOwnHandle_ref other)
		:  PythonOwnHandle<T>((T*)other.m_handle.m_instance)
    {
    	//stealing!
    	other.m_handle.m_instance = 0;
    }

    /**
     * Steals a reference from a PyReferenceSteal
     */
    inline PyReferenceSteal(PyReferenceSteal<T> &other)
		:  PythonOwnHandle<T>(other.release())
    {

    }

    /**
     * Steals a reference from a PyReferenceCreate
     */
    inline PyReferenceSteal( PyReferenceCreate<T>& other)
		:  PythonOwnHandle<T>(other.release())
    {

    }

    /**
     * Steals a reference from a temporary PythonOwnHandle
     */
    inline PyReferenceSteal<T>& operator = (typename PythonOwnHandle<T>::PythonOwnHandle_ref other)
    {

    	if (&other.m_handle != this) {
			  //next line has to be before decRef otherwise 'other.m_instance'
			  // can be deleted as an indirect code running when
			  // calling the destructor of 'this->m_instance'.
    		T *instance = (T*)other.m_handle.release();
    		decRef();
    		pointedChanged(instance);

    	}
    	return *this;
    }

    /**
     * Steals a reference from a PythonOwnHandle
     */
    inline PyReferenceSteal<T>& operator = (PyReferenceSteal<T> &other)
    {
    	if (&other != this) {
			  //next line has to be before decRef otherwise 'other.m_instance'
			  // can be deleted as an indirect code running when
			  // calling the destructor of 'this->m_instance'.
  		T *instance = (T*)other.release();
  		decRef();
  		pointedChanged(instance);    	}
    	return *this;
    }

    /**
     * Steals a reference from a PythonOwnHandle
     */
    inline PyReferenceSteal<T>& operator = (PyReferenceCreate<T>& other)
    {
		  //next line has to be before decRef otherwise 'other.m_instance'
		  // can be deleted as an indirect code running when
		  // calling the destructor of 'this->m_instance'.
		T *instance = (T*)other.release();
		decRef();
		pointedChanged(instance);
    	return *this;
    }

	template < class CONVERT_TO >
	friend inline const PyReferenceSteal<CONVERT_TO> &static_pyhcast(const PyReferenceSteal<T>& source)
	{
		//first make the compiler validate that the static_cast is ok
		T*dummy=0;
		(void)static_cast<CONVERT_TO*>(dummy);

		// Using reinterpret_cast to do the convertion for efficiency issues
		// We can do that because we already know the internal types can be converted
		return *(reinterpret_cast<const PyReferenceSteal<CONVERT_TO> *>(&source));
	}
};




/**
 * @class PyReferenceCreate
 *
 * This handle takes care of holding a new referenece to python objects.
 * PyReferenceCreate creates a new reference, which means:
 *     - It will immediately increase the reference count of the object
 *     - At some point it will decrease the reference count, possibly deleting the object.
 *     (but not if the original reference was not freed).
 */
template <typename T>
class PyReferenceCreate : public PythonOwnHandle<T>
{
	using PythonOwnHandle<T>::decRef;
	using PythonOwnHandle<T>::m_instance;
	using PythonOwnHandle<T>::pointedChanged;
	using PythonOwnHandle<T>::incRef;

public:
	/**
	 * Only calls the destructor from the base class
	 */
	inline ~PyReferenceCreate() {

	}

	inline PyReferenceCreate()
		: PythonOwnHandle<T>(0)
	{
	}

	/**
	 * Creates a new reference to PyObject.
	 * A new reference to the object is created which means that
	 * the original reference still has to be cleaned.
	 */
    inline explicit PyReferenceCreate(T *ptr)
    : PythonOwnHandle<T>(ptr)
    {

    	incRef();
    }

    /**
     * Creates a reference from a PyReferenceCreate
     */
    inline PyReferenceCreate(const PyReferenceCreate<T>& other)
    : PythonOwnHandle<T>((T*)other.m_instance)
    {
    	incRef();
    }

    /**
     * Creates a reference from a PyObjectHandle
     */
    inline PyReferenceCreate(const PyObjectHandle<T>& other)
    : PythonOwnHandle<T>((T*)other.m_instance)
    {
    	incRef();
    }


    /**
     * Creates a reference from a PyObjectHandle
     */
    inline PyReferenceCreate<T>& operator = (const PyObjectHandle<T>& other)
    {
    	if (&other != this) {
    		T *instance = (T*) other.m_instance;
    		other.incRef(); //This has to be done before the following decRef
    		decRef();
    		pointedChanged(instance);
    	}
    	return *this;
    }


	template < class CONVERT_TO >
	friend inline const PyReferenceCreate<CONVERT_TO> &static_pyhcast(const PyReferenceCreate<T>& source)
	{
		//validate that the static_cast is ok
		T*dummy;
		static_cast<CONVERT_TO*>(dummy);

		// Using reinterpret_cast to do the convertion for efficiency issues
		// We can do that because we already know the internal types can be converted
		return *(reinterpret_cast<const PyReferenceCreate<CONVERT_TO> *>(&source));
	}
};

/**
 * @class PyReferenceBorrow
 *
 * This handle takes care of pointing to python objects.
 * PyReferenceBorrow only holds a borrowed reference from a specific PythonOwnHandle.
 * Functions can borrow when recieving arguments, when returning arguments or when holding
 * internal variables (differently from some of the other handlers)
 * It should never be used after the original PythonOwnHandle was released.
 * In debug mode an exception of type std::logic_error will be thrown if trying to access
 * a reference throw an invalid PyObjectHandle.
 */
template <typename T>
class PyReferenceBorrow : public PyObjectHandle<T>
{
	using PyObjectHandle<T>::m_instance;

public:
	/**
	 * Only calls the destructor from the base class
	 */
	inline ~PyReferenceBorrow() {

	}

	inline PyReferenceBorrow()
	{
		m_instance = 0;
	}

	/**
	 * Borrows a reference to PyObject, this means that this
	 * reference is valid ONLY while the original reference is
	 * still valid.
	 */
    inline explicit PyReferenceBorrow(T *ptr)
    {
    	m_instance = convertToPyObject(ptr);
#ifdef ROBIN_PYOBJECTHANDLE_DEBUG
    	//Sorry
    	//cannot protect from references created from C pointers
    	//then creating a fake parent alive boolean so we get no
    	//errors
    	PyObjectHandle<T>::m_parentAlive = Handle<bool>(new bool(true));
#endif
    }

	/**
	 * Borrows a reference to PyObject, this means that this
	 * reference is valid ONLY while the original reference is
	 * still valid.
	 */
    inline PyReferenceBorrow(const PyObjectHandle<T>& other)
    {
    	m_instance = other.m_instance;
#ifdef ROBIN_PYOBJECTHANDLE_DEBUG
    	//connecting to parent, to make sure it is alive.
    	PyObjectHandle<T>::m_parentAlive = other.PyObjectHandle<T>::m_parentAlive;
#endif
    }


	/**
	 * Borrows a reference to PyObject, this means that this
	 * reference is valid ONLY while the original reference is
	 * still valid.
	 */
    inline PyReferenceBorrow<T>& operator = (const PyObjectHandle<T>& other)
    {
    	m_instance = other.m_instance;
#ifdef ROBIN_PYOBJECTHANDLE_DEBUG
    	//connecting to parent, to make sure it is alive.
    	PyObjectHandle<T>::m_parentAlive = other.PyObjectHandle<T>::m_parentAlive;
#endif
    	return *this;
    }

	template < class CONVERT_TO >
	friend inline const PyReferenceBorrow<CONVERT_TO> &static_pyhcast(const PyReferenceBorrow<T>& source)
	{
		//validate that the static_cast is ok
		T*dummy;
		static_cast<CONVERT_TO*>(dummy);

		// Using reinterpret_cast to do the convertion for efficiency issues
		// We can do that because we already know the internal types can be converted
		return *(reinterpret_cast<const PyReferenceBorrow<CONVERT_TO> *>(&source));
	}
};





}; //namespace Python
}; //namespace Robin


#endif
