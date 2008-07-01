// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * typeofargument.h
 *
 * @par TITLE
 * Argument Declaration
 *
 * Declares the Argument class, which is the basic building
 * block of prototypes in the reflection. Unlike the 
 * <classref>Adapter</classref> (which is an interface exported by the
 * Frontends Framework), the TypeOfArgument is a generic object and it's
 * independant of the scripting environment.
 */

#ifndef ROBIN_TYPEOFARGUMENT_H
#define ROBIN_TYPEOFARGUMENT_H

// STL includes
#include <vector>
#include <exception>

// Pattern includes
#include <pattern/handle.h>
#include <pattern/handle_map.h>

// Package includes
#include "argumentsbuffer.h"
#include "callable.h"

namespace Robin {

class Class;
class EnumeratedType;
class Adapter;

/**
 * @par ENUM
 * TypeCategory
 */
enum TypeCategory
{
    TYPE_CATEGORY_INTRINSIC,
    TYPE_CATEGORY_EXTENDED,
    TYPE_CATEGORY_USERDEFINED
};

/**
 * @par ENUM
 * TypeSpec
 */
enum TypeSpec
{
    TYPE_INTRINSIC_INT,
    TYPE_INTRINSIC_UINT,
    TYPE_INTRINSIC_LONG,
    TYPE_INTRINSIC_LONG_LONG,
    TYPE_INTRINSIC_ULONG,
    TYPE_INTRINSIC_ULONG_LONG,
    TYPE_INTRINSIC_CHAR,
    TYPE_INTRINSIC_SCHAR,
    TYPE_INTRINSIC_UCHAR,
    TYPE_INTRINSIC_SHORT,
    TYPE_INTRINSIC_USHORT,
    TYPE_INTRINSIC_LONGLONG,
    TYPE_INTRINSIC_ULONGLONG,
    TYPE_INTRINSIC_FLOAT,
    TYPE_INTRINSIC_DOUBLE,
    TYPE_INTRINSIC_BOOL,
    TYPE_EXTENDED_VOID,
    TYPE_EXTENDED_CSTRING,
	TYPE_EXTENDED_PASCALSTRING,
	TYPE_EXTENDED_ELEMENT,
    TYPE_USERDEFINED_OBJECT,
    TYPE_USERDEFINED_ENUM
};

/**
 * @par STRUCT
 * Type
 *
 * Represents a basic type (without modifiers).
 */
struct Type
{
    TypeCategory category;
    TypeSpec     spec;
    Handle<Class> objclass;  /* applies when spec==TYPE_USERDEFINED_OBJECT */
    Handle<EnumeratedType> objenum;  /* when spec==TYPE_USERDEFINED_ENUM */

    ~Type();
};

/**
 * @class TypeOfArgument
 * @nosubgrouping
 *
 * Represents a formal argument passed to a function,
 * or a formal value returned by it. One TypeOfArgument instance exists
 * for every possible type in the entire reflection - thus reducing
 * the number of instances considerably.<br />
 * The TypeOfArgument does not only hold the type information of the
 * argument, but can also perform approperiate Translation, by
 * delegating the calls to an <classref>Adapter</classref> which is
 * associated with it.
 */
class TypeOfArgument
{
public:
    /**
     * @name Constructors
     */

    //@{
    TypeOfArgument(TypeCategory category, TypeSpec spec, bool borrowed = false);
    TypeOfArgument(Handle<Class>, bool borrowed = false);
    TypeOfArgument(Handle<EnumeratedType>, bool borrowed = false);

    ~TypeOfArgument();

    //@}
    /**
     * @name Access
     */

    //@{
    Type basetype() const;
	bool isPointer() const;
	bool isReference() const;
    bool isBorrowed() const;
	Handle<TypeOfArgument> pointer() const;
	const TypeOfArgument& pointed() const;
    //@}

    /**
     * @name Translation
     *
     * Methods that perform Active Translation
     * by forwarding the call to the Adapter.
	 * These operations help connect between the Internal
	 * Reflection and the Frontends Framework; the TypeOfArgument stores
	 * an <classref>Adapter</classref> which is implemented by the
	 * frontend, and uses it to perform the translations.
	 */

    //@{
    void assignAdapter(Handle<Adapter> adapter);

    scripting_element get(basic_block rval);
    void put(ArgumentsBuffer& argsbuf, scripting_element value);
	//@}

	static Pattern::HandleMap<TypeOfArgument> handleMap;

private:
    Type m_basetype;
    int  m_redirection_degree;
    bool m_borrowed;
    std::vector<int> m_array_dimensions;

    Handle<Adapter> m_adapter;

	// - pointer
	mutable Handle<TypeOfArgument> m_cache_pointer;
	const TypeOfArgument *m_pointed;
};


/**
 * @class UnsupportedInterfaceException
 * @nosubgrouping
 *
 * Thrown when trying to perform translations using a
 * <classref>TypeOfArgument</classref> which does not have an
 * <classref>Adapter</classref> associated with it.<br />
 * Also, this excpetion is thrown by a frontend when the type of an
 * element cannot be detected, or when an <classref>Adapter</classref>
 * is not available for this object type.
 */
class UnsupportedInterfaceException : public std::exception
{
public:
    const char *what() const throw() { return "unsupported interface"; }
};


} // end of namespace Robin

#endif
//@}
