// -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * intrinsic_type_arguments.h
 *
 * @par TITLE
 * Arguments of Intrinsic Types
 *
 * Defines the basic instances of TypeOfArgument in which the
 * base type is intrinsic (category == TYPE_CATEGORY_INTRINSIC or
 * category == TYPE_CATEGORY_EXTENDED) and no redirection / reference is
 * used.
 *
 * @par PUBLIC-LITERALS
 *
 * <ul><li>ArgumentInt</li>
 *     <li>ArgumentLong</li>
 * </ul>
 */

#include <pattern/handle.h>
#include "typeofargument.h"

namespace Robin {

	/**
	 * \@VARIABLE DECLARATIONS
	 */
	extern Handle<TypeOfArgument> ArgumentInt;
	extern Handle<TypeOfArgument> ArgumentLong;
	extern Handle<TypeOfArgument> ArgumentLongLong;
	extern Handle<TypeOfArgument> ArgumentShort;
	extern Handle<TypeOfArgument> ArgumentUInt;
	extern Handle<TypeOfArgument> ArgumentULong;
	extern Handle<TypeOfArgument> ArgumentULongLong;
	extern Handle<TypeOfArgument> ArgumentUShort;
	extern Handle<TypeOfArgument> ArgumentChar;
	extern Handle<TypeOfArgument> ArgumentUChar;
	extern Handle<TypeOfArgument> ArgumentSChar;
	extern Handle<TypeOfArgument> ArgumentFloat;
	extern Handle<TypeOfArgument> ArgumentDouble;
	extern Handle<TypeOfArgument> ArgumentBoolean;
	extern Handle<TypeOfArgument> ArgumentVoid;
	extern Handle<TypeOfArgument> ArgumentCString;
	extern Handle<TypeOfArgument> ArgumentPascalString;

	extern Handle<TypeOfArgument> ArgumentScriptingElement;

} // end of namespace Robin
