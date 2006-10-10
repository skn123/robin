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

// Package includes
#include "typeofargument.h"
#include "callable.h"


namespace Robin {


/**
 * Stores an address pointing to some data, which may vary by
 * type.
 */
class Address
{
public:
    Address(Handle<TypeOfArgument> pointedType, void *address);

    /**
     * @name Access
     */
    //@{
    Handle<TypeOfArgument> getPointedType() const;
    Handle<TypeOfArgument> getPointerType() const;
    void *asPointer() const;
    //@}

    /**
     * @name Referencing
     */
    //@{
    scripting_element dereference() const;
    //@}

private:
    Handle<TypeOfArgument> m_pointedType;
    void *m_address;
};


} // end of namespace Robin


#endif
