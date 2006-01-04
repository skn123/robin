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

Handle<TypeOfArgument> ArgumentInt (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_INT));
Handle<TypeOfArgument> ArgumentLong(new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_LONG));
Handle<TypeOfArgument> ArgumentLongLong
                                   (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_LONG_LONG));
Handle<TypeOfArgument> ArgumentShort
                                   (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_SHORT));
Handle<TypeOfArgument> ArgumentUInt(new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_UINT));
Handle<TypeOfArgument> ArgumentULong
                                   (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_ULONG));
Handle<TypeOfArgument> ArgumentULongLong
                                   (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_ULONG_LONG));
Handle<TypeOfArgument> ArgumentUShort
                                   (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_USHORT));
Handle<TypeOfArgument> ArgumentChar(new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_CHAR));
Handle<TypeOfArgument> ArgumentSChar
                                   (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_SCHAR));
Handle<TypeOfArgument> ArgumentUChar
                                   (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_UCHAR));
Handle<TypeOfArgument> ArgumentFloat
                                   (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_FLOAT));
Handle<TypeOfArgument> ArgumentDouble
                                   (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_DOUBLE));
Handle<TypeOfArgument> ArgumentBoolean
                                   (new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_BOOL));
Handle<TypeOfArgument> ArgumentVoid(new TypeOfArgument(TYPE_CATEGORY_INTRINSIC,
						            TYPE_INTRINSIC_BOOL));

Handle<TypeOfArgument> ArgumentCString
                                   (new TypeOfArgument(TYPE_CATEGORY_EXTENDED,
						            TYPE_EXTENDED_CSTRING));
Handle<TypeOfArgument> ArgumentPascalString
                                   (new TypeOfArgument(TYPE_CATEGORY_EXTENDED,
						            TYPE_EXTENDED_PASCALSTRING));

Handle<TypeOfArgument> ArgumentScriptingElement
                                   (new TypeOfArgument(TYPE_CATEGORY_EXTENDED,
						            TYPE_EXTENDED_ELEMENT));

#endif

} // end of namespace Robin

