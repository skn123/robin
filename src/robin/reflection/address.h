// -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * @file
 *
 * @par SOURCE
 * address.h
 *
 * @par TITLE
 * Address Object
 *
 * @par PACKAGE
 * Robin
 *
 * Implements pointer handling, storing addresses, and dereferencing them.
 */

#ifndef ROBIN_REFLECTION_ADDRESS_H
#define ROBIN_REFLECTION_ADDRESS_H

// STL includes
#include <exception>

// Package includes
#include "robintype.h"
#include "callable.h"


namespace Robin {


/**
 * Stores an address pointing to some data, which may vary by
 * type.
 */
class Address
{
public:
    Address(Handle<RobinType> pointedType, void *address);

    /**
     * @name Access
     */
    //@{
    Handle<RobinType> getPointedType() const;
    Handle<RobinType> getPointerType() const;
    void *asPointer() const;
    //@}

    /**
     * @name Referencing
     */
    //@{
    scripting_element dereference() const;
    scripting_element dereference(int subscript) const;
    Handle<Address> reference();
    //@}

private:
    Handle<RobinType> m_pointedType;
    void *m_address;
};


/**
 * Thrown when attempting to dereference a NULL pointer.
 */
class NullDereferenceException : std::exception
{
public:
    ~NullDereferenceException() throw() { }

    const char *what() const throw() { return "null pointer dereference"; }
};


} // end of namespace Robin


#endif
