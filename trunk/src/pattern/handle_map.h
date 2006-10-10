// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @par SOURCE
 * handles.h
 *
 * @par TITLE
 * Mapping C++ Pointers to Handles
 *
 * It is often the case where an object acquires a pointer,
 * but is required to pass a handle. This maps assists in
 * registering global object handles.
 */

#ifndef PATTERN_HANDLE_MAP_H
#define PATTERN_HANDLE_MAP_H


#include <map>
#include "handle.h"


namespace Pattern {


/**
 * Maps pointers to Handles. A Handle<T> is first registered
 * via the registerHandle() method, and can then be recalled using
 * the acquire() method.
 */
template < class T >
class HandleMap
{
public:
	void registerHandle(Handle<T> handle);
	Handle<T> acquire(const T *pointer) const;
	Handle<T> acquire(const T& reference) const;

private:
	typedef std::map<const T*, Handle<T> > mapping;
	mapping m_pointer2handle;
};


/**
 * Registers a handle-controlled object.
 */
template < class T >
void HandleMap<T>::registerHandle(Handle<T> handle)
{
	m_pointer2handle[&*handle] = handle;
}

/**
 * Extracts a Handle for a previously registered object.
 */
template < class T >
Handle<T> HandleMap<T>::acquire(const T *pointer) const
{
	typename mapping::const_iterator look = m_pointer2handle.find(pointer);

	if (look != m_pointer2handle.end()) {
		return look->second;
	}
	else {
		return Handle<T>();
	}
}

/**
 * Extracts a Handle for a previously registered object.
 */
template < class T >
Handle<T> HandleMap<T>::acquire(const T& reference) const
{
	return acquire(&reference);
}



} // end of namespace Pattern


#endif
