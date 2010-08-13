// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/address.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "address.h"


namespace Robin {

/**
 * Construct Address from the type being referenced and C pointer.
 *
 * @param pointedTo the type of the datum referenced by the address
 * @param address a C++ void pointer
 */
Address::Address(Handle<RobinType> pointedType, void *address)
    : m_pointedType(pointedType), m_address(address)
{
}

/**
 * Returns the type of the datum referenced by the Address.
 * For example, if the address is a pointer to an integer, this
 * call will return 'int' (ArgumentInt).
 * @return a RobinType corresponding to the pointed type
 */
Handle<RobinType> Address::getPointedType() const
{
    return m_pointedType;
}

/**
 * Returns the pointer type which this Address represents.
 * For example, if the address is a pointer to an integer, this
 * call will return 'int*'.
 *
 * @return a RobinType corresponding to the pointer type
 */
Handle<RobinType> Address::getPointerType() const
{
    return m_pointedType->pointer();
}

/**
 * Returns the address stored as a void *.
 */
void *Address::asPointer() const
{
    return m_address;
}


/**
 * Extracts the datum stored in the address.
 *
 * @return a scripting_element containing the referenced value
 */
scripting_element Address::dereference() const
{
    if (m_address == NULL) throw NullDereferenceException();

    basic_block datum = *((basic_block*)m_address);
    return m_pointedType->get(datum);
}

/**
 * Extracts the datum stored in the i-th index of an array.
 *
 * @param subscript 0-based index to an array which starts at the
 *   address this Address object represents
 * @return a scripting_element containing the referenced value
 */
scripting_element Address::dereference(int subscript) const
{
    if (m_address == NULL) throw NullDereferenceException();

    basic_block datum = ((basic_block*)m_address)[subscript];
    return m_pointedType->get(datum);
}

/**
 * Creates a pointer to the pointer held in the Address.
 *
 * @return a new Address object with one redirection level more.
 */
Handle<Address> Address::reference()
{
    return Handle<Address>(new Address(getPointerType(),
                                       &m_address));
}


} // end of namespace Robin
