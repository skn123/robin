// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @par SOURCE
 * handles.h
 *
 * @par TITLE
 * Generic Implementation of Handles
 *
 * Handles (or smart pointers) keep track of memory allocation
 * and deallocation using a simple reference count.
 */

#ifndef PATTERN_HANDLES_H
#define PATTERN_HANDLES_H

#include <stdio.h>

/**
 * @class Handle
 * @nosubgrouping
 *
 * Implements a smart pointer. Upon copying the Handle,
 * the reference count is incremented; upon destruction of one, the
 * count is decremeneted. When the reference count reaches zero, no
 * more instances of the pointed object are referenced, and so the 
 * object is deleted.
 */ 
template <class T>
class Handle
{
public:
    Handle() : m_instance(0), m_refcapsule(new int(1)) { }
    explicit Handle(T *ptr) : m_instance(ptr), m_refcapsule(new int(1)) { }
    Handle(T *ptr, int *capsule) : m_instance(ptr), m_refcapsule(capsule)
       { incRef(); }
    Handle(const Handle<T>& other);

    Handle<T>& operator = (const Handle<T>& other);
    bool operator == (const Handle<T>& other) const;

    ~Handle() { decRef(); }

    /**
     * @name Pointer-like Interface
     */

    //@{
    inline T * operator -> () const { return m_instance; }
    inline T & operator * () const { return *m_instance; }

    inline operator bool() const { return m_instance != 0; }

#ifdef __TODO_autoconf
	template < class CONVERT_TO, class S >
	friend Handle<CONVERT_TO> static_hcast(const Handle<S>& source)
#else
	template < class CONVERT_TO >
	friend Handle<CONVERT_TO> static_hcast(const Handle<T>& source)
#endif
	{
		return Handle<CONVERT_TO>(static_cast<CONVERT_TO*>(source.m_instance),
								  source.m_refcapsule);
	}
	//@}

protected:
    inline void release();
    inline void incRef() { ++(*m_refcapsule); }
    inline void decRef() { if (--(*m_refcapsule) <= 0) release(); }

    T * m_instance;
    int * m_refcapsule;
};

template < class T >
Handle<T>::Handle(const Handle<T>& other)
    : m_instance(other.m_instance), m_refcapsule(other.m_refcapsule)
{
    incRef();
}

template < class T >
Handle<T>& Handle<T>::operator = (const Handle<T>& other)
{
    if (&other != this) {
	decRef();
	m_instance = other.m_instance;
	m_refcapsule = other.m_refcapsule;
	incRef();
    }
    return *this;
}

template < class T >
bool Handle<T>::operator == (const Handle<T>& other) const
{
	return (m_instance == other.m_instance);
}

template < class T >
void Handle<T>::release()
{
    delete m_refcapsule;
    if (m_instance) delete m_instance;
}

#endif
