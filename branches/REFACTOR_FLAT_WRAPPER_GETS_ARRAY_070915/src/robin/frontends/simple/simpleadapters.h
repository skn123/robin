// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/simple/simpleadapters.h
 *
 * @par TITLE
 * Simple Adapter Implementations
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_SIMPLE_FRONTEND_ADAPTERS_H
#define ROBIN_SIMPLE_FRONTEND_ADAPTERS_H

/**
 * \@INCLUDES
 */

// Package includes
#include <robin/frontends/adapter.h>
#include <robin/reflection/class.h>
#include <robin/reflection/enumeratedtype.h>
#include "elements.h"


namespace Robin {

/**
 * @class SimpleAdapter
 * @nosubgrouping
 *
 * A template for intrinsic-type arguments which are
 * smaller than the size of a pointer. The template arguments are the
 * C type and the Simple::Element derivative representing the type in
 * the Simple interpreter.
 */
template < class CType, class SimpleType >
class SimpleAdapter : public Adapter
{
public:
    virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
    virtual scripting_element get(basic_block data);
};

/**
 * @class SimpleAdapterWithType
 * @nosubgrouping
 *
 * A template for intrinsic-type arguments which are
 * smaller than the size of a pointer. The template arguments are the
 * C type and the Simple::Element derivative representing the type in
 * the Simple interpreter. This adapter performs an explicit cast
 * from element->value to CType.
 */
template < class CType, class SimpleType >
class SimpleAdapterWithCast : public Adapter
{
public:
    virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
    virtual scripting_element get(basic_block data);
};

/**
 * @class SimpleLargeAdapter
 * @nosubgrouping
 *
 * A template for intrinsic-type arguments which are
 * larger than the size of a pointer. The template arguments are the
 * C type and the Simple::Element derivative representing the type in
 * the Simple interpreter.
 */
template < class CType, class SimpleType >
class SimpleLargeAdapter : public Adapter
{
public:
    virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
    virtual scripting_element get(basic_block data);
};

/**
 * @class SimpleCStringAdapter
 * @nosubgrouping
 *
 * An adapter for transferring plain strings. A pointer
 * to the string (char*) is used for communication, so the string is
 * assumed to be null-terminated.
 */
class SimpleCStringAdapter : public Adapter
{
public:
    virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
    virtual scripting_element get(basic_block data);
};

/**
 * @class SimplePascalStringAdapter
 * @nosubgrouping
 *
 * An adapter for transferring Pascal-style strings. These strings are
 * prefixed by an integer containing the string length, so the string may
 * contain embedded nulls.
 */
class SimplePascalStringAdapter : public Adapter
{
public:
    virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
    virtual scripting_element get(basic_block data);
};

/**
 * @class SimpleInstanceAdapter
 * @nosubgrouping
 *
 * Used to transfer and receive Instance objects. The adapter stores 
 * the class of which type objects are being transferred; one instance 
 * of this adapter exists for each transferrable class.
 */
class SimpleInstanceAdapter : public Adapter
{
public:
	SimpleInstanceAdapter(Handle<Class> domain);

    virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
    virtual scripting_element get(basic_block data);

private:
	Handle<Class> m_domain;
};

/**
 * @class SimpleAddressAdapter
 * @nosubgrouping
 *
 * Used to transfer and receive Address objects. This way pointers to
 * primitive types and pointers to pointers can be translated. The 
 * adapter stores the type of datum being addressed; one instance of
 * this adapter exists for each handled pointer type.
 */
class SimpleAddressAdapter : public Adapter
{
public:
	SimpleAddressAdapter(Handle<TypeOfArgument> pointedType);

	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
    virtual scripting_element get(basic_block data);

private:
	Handle<TypeOfArgument> m_domain;
};

/**
 * @class SimpleEnumeratedAdapter
 * @nosubgrouping
 *
 * Used to transfer and receive enumerated data types
 * in the form of SimpleEnumeratedConstantElement-s
 * which are a part of the Simple Interpreter's standard extension.
 * Enumerated values are always transferred as ints; when they are
 * received, the approperiate type tag is attached making them
 * fully enumerated constants.
 * One instance of this adapter exists for each enumerated type.
 */
class SimpleEnumeratedAdapter : public Adapter
{
public:
	SimpleEnumeratedAdapter(Handle<EnumeratedType> domain);

	virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
    virtual scripting_element get(basic_block data);

private:
	Handle<EnumeratedType> m_domain;
};



} // end of namespace Robin

#include "simpleadapters.inl"

#endif
