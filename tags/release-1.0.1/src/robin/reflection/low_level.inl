// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 */

#include "../config.h"

namespace Robin {

template < class CType, int PadSize > 
struct Padded { char pad[PadSize]; CType value; };
template < class CType > 
struct Padded<CType, 0> { CType value; };

#ifdef _MSC_VER
#define ANSI_ALIGNMENT(CType) 0 /* MSC cannot understand the expr. below */
#else
#define ANSI_ALIGNMENT(CType) \
	((MACH_ENDIAN == BIG) ? (sizeof(basic_block) - sizeof(CType)) : 0)
#endif

/**
 * (internal)
 * A union used to reinterpret values returned from functions.
 * Some value returned as a basic_block is assigned to the 'data' field,
 * and expected returned value in the desired type is extracted as
 * 'k.value' field.
 */
template < class CType >
union Holder {
	basic_block                          data;
	Padded<CType,ANSI_ALIGNMENT(CType)>  k;
};

/**
 * (internal)
 * Casts a data element returned from a reflected function call into the
 * specified CType, applying ANSI calling convention rules.
 * This function is very useful when programming Adapter objects.
 */
template < class CType >
CType LowLevel::reinterpret_lowlevel(basic_block data)
{
	Holder<CType> holder;
	holder.data = data;
	return holder.k.value;
}

} // end of namespace Robin
