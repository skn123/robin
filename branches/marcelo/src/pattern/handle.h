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

#include <robin/debug/assert.h>


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

    /**
     * An automatic conversion to another type of handle
     * if the pointed types are related
     */
    template <typename S>
    inline operator Handle<S>() const
    {
		//first make the compiler validate that the cast is ok
		T*dummy=0;
		(void)static_cast<S*>(dummy);

    	return Handle<S>(reinterpret_cast<const Handle<S>&>(*this));
    }




    Handle<T>& operator = (const Handle<T>& other);
    bool operator == (const Handle<T>& other) const;
    bool operator != (const Handle<T>& other) const;
    bool operator<(const Handle<T> &other)const;
    /**
     * release in the same sense that std::auto_ptr.
     * Means that it will return the pointer and no longer be responsible for it
     * It must be called only when there is only one handle to the address.
     */
    inline T *release();

    /**
     * It returns a pointer to the internal object, of course the pointer
     * does not increase the refcount of the object so the user should
     * take care that the pointer is not released.
     */
    inline T *pointer() const;

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
    inline void incRef() { ++(*m_refcapsule);  }
    inline void decRef()
    {
    	(*m_refcapsule)--;
    	if ( *m_refcapsule <= 0)
		{
            delete m_refcapsule;
            if (m_instance) delete m_instance;
		}
    }

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
    	int *newCapsule = other.m_refcapsule;
    	(*newCapsule)++;
    	T * newInstance = other.m_instance;
		decRef();  // carefull when changing the order of the operation
				    // in relation to decRef()
		            //decref might call arbitrary code which might
					// even call the destructor of 'other'
		m_instance = newInstance;
		m_refcapsule = newCapsule;
    }
    return *this;
}


template < class T >
bool Handle<T>::operator != (const Handle<T>& other) const
{
	return (m_instance != other.m_instance);
}


template < class T >
bool Handle<T>::operator == (const Handle<T>& other) const
{
	return (m_instance == other.m_instance);
}

template <class T>
inline bool Handle<T>::operator<(const Handle<T> &other) const
{
	return m_instance < other.m_instance;
}

template < class T >
T* Handle<T>::release()
{
	assert_true(*m_refcapsule < 2); // This function can be called if there is no other handle
							   // which will delete the pointer

	T*instance = m_instance;
	m_instance = 0;
	return instance;
}

template < class T >
T* Handle<T>::pointer() const
{
	return m_instance;
}

#endif
