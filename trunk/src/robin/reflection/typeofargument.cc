// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/typeofargument.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "typeofargument.h"
#include "class.h"
#include "enumeratedtype.h"
#include "../frontends/adapter.h"


namespace Robin {




Type::~Type() 
{
    // This destructor is here only to keep instantiation of Handle<Adapter>
    // out of the header.
}





/**
 * Builds an intrinsic argument type (category==
 * TYPE_CATEGORY_INTRINSIC or category==TYPE_CATEGORY_EXTENDED).
 * It assigns no redirection, reference or array indicators.
 */
TypeOfArgument::TypeOfArgument(TypeCategory category, TypeSpec spec, bool borrowed /* = false */)
	: m_redirection_degree(0), m_borrowed(borrowed)
{
    m_basetype.category = category;
    m_basetype.spec = spec;
}

/**
 * Builds an argument of a class type. Assigns the
 * value TYPE_CATEGORY_USERDEFINED to category and TYPE_USERDEFINED_OBJECT
 * to spec.
 */
TypeOfArgument::TypeOfArgument(Handle<Class> classtype, bool borrowed /* = false */)
	: m_redirection_degree(0), m_borrowed(borrowed)
{
    m_basetype.category = TYPE_CATEGORY_USERDEFINED;
    m_basetype.spec = TYPE_USERDEFINED_OBJECT;
    m_basetype.objclass = classtype;
}

/**
 * Builds an argument of an enumerated type. assigns the
 * value TYPE_CATEGORY_USERDEFINED to category and TYPE_USERDEFINED_ENUM
 * to spec.
 */
TypeOfArgument::TypeOfArgument(Handle<EnumeratedType> enumtype, bool borrowed /* = false */)
	: m_redirection_degree(0), m_borrowed(borrowed)
{
    m_basetype.category = TYPE_CATEGORY_USERDEFINED;
    m_basetype.spec = TYPE_USERDEFINED_ENUM;
    m_basetype.objenum = enumtype;
}

/**
 * Releases resources associated with the argument type,
 * such as the <classref>Adapter</classref>.
 */
TypeOfArgument::~TypeOfArgument()
{
    // This destructor is here only to keep instantiation of Handle<Adapter>
    // out of the header.
}



/**
 * Returns a <classref>Type</classref> structures which
 * helps to identify the type this TypeOfArgument object denotes.
 * This information is useful for frontend writers.
 */
Type TypeOfArgument::basetype() const
{
    return m_basetype;
}

/**
 * Checks whether this type is a pointer type.
 *
 * @return true if this is a pointer type
 */
bool TypeOfArgument::isPointer() const
{
	return (m_redirection_degree > 0);
}

/**
 * Checks whether this type is a reference-to-object type.
 *
 * @return true if this is a reference type
 */
bool TypeOfArgument::isReference() const
{
	return  (m_basetype.category == TYPE_CATEGORY_USERDEFINED &&
	         m_basetype.spec == TYPE_USERDEFINED_OBJECT &&
	         this == &*(m_basetype.objclass->getRefArg()));
}

/**
 * Checks whether this type is borrowed
 *
 * @return true if this is a borrowed type
 */
bool TypeOfArgument::isBorrowed() const
{
	return m_borrowed;
}

/**
 * Returns a handle to a TypeOfArgument which represents a pointer to the
 * specified type. Two consecutive calls to pointer() will always return the
 * same object.
 */
Handle<TypeOfArgument> TypeOfArgument::pointer() const
{
	if (!m_cache_pointer) {
		m_cache_pointer = Handle<TypeOfArgument>(new TypeOfArgument(*this));
		m_cache_pointer->m_redirection_degree++;
		m_cache_pointer->m_adapter = Handle<Adapter>();
		m_cache_pointer->m_pointed = this;
	}
	return m_cache_pointer;
}


/**
 * Only valid for pointer types; returns the type for which this
 * type is a pointer. For example, 'int*'->pointed() will return 'int'.
 */
const TypeOfArgument& TypeOfArgument::pointed() const
{
	return *m_pointed;
}


/**
 * Associates an <classref>Adapter</classref> object
 * with the argument. This Adapter should know how to translate the
 * <b>base type</b> alone - any redirection/dereferencing is done
 * by the TypeOfArgument::get and TypeOfArgument::put methods.
 */
void TypeOfArgument::assignAdapter(Handle<Adapter> adapter)
{
    m_adapter = adapter;
}

/**
 * @par DESCIPRTION
 * Translates a return value using the associated 
 * Adapter. The return value is a scripting_element in the terms
 * of the active frontend.
 */
scripting_element TypeOfArgument::get(basic_block return_value)
{
    if (m_adapter)
		return m_adapter->get(return_value);
    else
		throw UnsupportedInterfaceException();
}

/**
 * Translate an argument given in the terms of the
 * active scripting environment frontend to C standards and pushes
 * it on the <classref>ArgumentsBuffer</classref>.
 */
void TypeOfArgument::put(ArgumentsBuffer& argsbuf, scripting_element value)
{
    if (m_adapter)
		m_adapter->put(argsbuf, value);
    else
		throw UnsupportedInterfaceException();
}


Pattern::HandleMap<TypeOfArgument> TypeOfArgument::handleMap;



} // end of namespace Robin
