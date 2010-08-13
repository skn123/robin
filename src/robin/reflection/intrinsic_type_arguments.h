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
 * Defines the basic instances of RobinType in which the
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
#include "robintype.h"

namespace Robin {

	/**
	 * \@VARIABLE DECLARATIONS
	 */
	extern Handle<RobinType> ArgumentInt;
	extern Handle<RobinType> ArgumentLong;
	extern Handle<RobinType> ArgumentLongLong;
	extern Handle<RobinType> ArgumentShort;
	extern Handle<RobinType> ArgumentUInt;
	extern Handle<RobinType> ArgumentULong;
	extern Handle<RobinType> ArgumentULongLong;
	extern Handle<RobinType> ArgumentUShort;
	extern Handle<RobinType> ArgumentChar;
	extern Handle<RobinType> ArgumentUChar;
	extern Handle<RobinType> ArgumentSChar;
	extern Handle<RobinType> ArgumentFloat;
	extern Handle<RobinType> ArgumentDouble;
	extern Handle<RobinType> ArgumentBoolean;
	extern Handle<RobinType> ArgumentVoid;
	extern Handle<RobinType> ArgumentCString;
	extern Handle<RobinType> ArgumentPascalString;

	extern Handle<RobinType> ArgumentScriptingElementNewRef;
	extern Handle<RobinType> ArgumentScriptingElementBorrowedRef;

} // end of namespace Robin
