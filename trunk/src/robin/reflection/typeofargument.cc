// -*- C++ -*-

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
TypeOfArgument::TypeOfArgument(TypeCategory category, TypeSpec spec)
{
    m_basetype.category = category;
    m_basetype.spec = spec;
}

/**
 * Builds an argument of a class type. Assigns the
 * value TYPE_CATEGORY_USERDEFINED to category and TYPE_USERDEFINED_OBJECT
 * to spec.
 */
TypeOfArgument::TypeOfArgument(Handle<Class> classtype)
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
TypeOfArgument::TypeOfArgument(Handle<EnumeratedType> enumtype)
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


} // end of namespace Robin
