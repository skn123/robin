// -*- C++ -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/intrinsic_type_arguments.cc
 *
 * @par PACKAGE
 * Robin
 *
 * @par TITLE
 * Intrinsic Type Argument Constants
 */

#include "intrinsic_type_arguments.h"


namespace Robin {

#ifndef __doxygen

Handle<RobinType> ArgumentInt (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_INT,RobinType::constReferenceKind));
Handle<RobinType> ArgumentLong(RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_LONG,RobinType::constReferenceKind));
Handle<RobinType> ArgumentLongLong
                                   (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_LONG_LONG,RobinType::constReferenceKind));
Handle<RobinType> ArgumentShort
                                   (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_SHORT,RobinType::constReferenceKind));
Handle<RobinType> ArgumentUInt(RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_UINT,RobinType::constReferenceKind));
Handle<RobinType> ArgumentULong
                                   (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_ULONG,RobinType::constReferenceKind));
Handle<RobinType> ArgumentULongLong
                                   (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_ULONG_LONG,RobinType::constReferenceKind));
Handle<RobinType> ArgumentUShort
                                   (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_USHORT,RobinType::constReferenceKind));
Handle<RobinType> ArgumentChar(RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_CHAR,RobinType::constReferenceKind));
Handle<RobinType> ArgumentSChar
                                   (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_SCHAR,RobinType::constReferenceKind));
Handle<RobinType> ArgumentUChar
                                   (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_UCHAR,RobinType::constReferenceKind));
Handle<RobinType> ArgumentFloat
                                   (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_FLOAT,RobinType::constReferenceKind));
Handle<RobinType> ArgumentDouble
                                   (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_DOUBLE,RobinType::constReferenceKind));
Handle<RobinType> ArgumentBoolean
                                   (RobinType::create_new(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_BOOL,RobinType::constReferenceKind));
Handle<RobinType> ArgumentVoid(RobinType::create_new(TYPE_CATEGORY_EXTENDED,
						            TYPE_EXTENDED_VOID,RobinType::constReferenceKind));

Handle<RobinType> ArgumentCString
                                   (RobinType::create_new(TYPE_CATEGORY_EXTENDED,
						            TYPE_EXTENDED_CSTRING,RobinType::constReferenceKind));
Handle<RobinType> ArgumentPascalString
                                   (RobinType::create_new(TYPE_CATEGORY_EXTENDED,
						            TYPE_EXTENDED_PASCALSTRING,RobinType::constReferenceKind));

Handle<RobinType> ArgumentScriptingElementNewRef
                                   (RobinType::create_new(TYPE_CATEGORY_EXTENDED,
						            TYPE_EXTENDED_ELEMENT, "*scripting_element",RobinType::regularKind));

Handle<RobinType> ArgumentScriptingElementBorrowedRef
                                   (RobinType::create_new(TYPE_CATEGORY_EXTENDED,
						            TYPE_EXTENDED_ELEMENT, "&scripting_element",RobinType::regularKind,/* borrowed = */true));

#endif

} // end of namespace Robin

