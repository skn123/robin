// -*- C++ -*-

/**
 * @file
 *
 * @par SOURCE
 * low_level.h
 *
 * @par TITLE
 * Low Level call in C.
 *
 * @par PUBLIC-FUNCTIONS
 *
 * <ul><li>call_lowlevel</li>
 *     <li>call_lowlevel_void</li>
 * </ul>
 */

#ifndef ROBIN_LOWLEVEL_H
#define ROBIN_LOWLEVEL_H

#include "argumentsbuffer.h"

#define IS_ARGUMENT_LIMIT
namespace Robin {

/**
 * \@TYPES
 */

typedef void *symbol;

#ifdef IS_ARGUMENT_LIMIT
typedef basic_block (*external)(basic_block, basic_block, basic_block,
                basic_block, basic_block, basic_block,
                basic_block, basic_block, basic_block,
                basic_block, basic_block, basic_block);
typedef void (*externalv)(basic_block, basic_block, basic_block,
                 basic_block, basic_block, basic_block,
                 basic_block, basic_block, basic_block,
                 basic_block, basic_block, basic_block);
#else
typedef basic_block (*external)(void);
typedef void (*externalv)(void);
#endif

/**
 * @class LowLevel
 * @nosubgroups
 *
 * <p>This is the low level interface to the actual functions to
 * be called.</p>
 * <p>Using the low level calls, functions are given actual memory
 * blocks, and not their ArgumentBuffer representation, and then
 * call the relevant function, finally returning the actual return
 * type (adapted back to the original type or just as a machine word).</p>
 */
class LowLevel {

public:

	/**
	 * @name Call
	 */

	//@{
	virtual basic_block call_lowlevel(symbol function, const basic_block* args, size_t argsCount) const;
	
	virtual void call_lowlevel_void(symbol function, const basic_block* args, size_t argsCount) const;

	//@}
	/**
	 * @name Utils
	 */

	//@{
	template < class T >
	static T reinterpret_lowlevel(basic_block value);

	//@}
	/**
	 * @name Destructor
	 */

	//@{
	virtual ~LowLevel() {}

	//@}

};

} // end of namespace Robin

#include "low_level.inl"

#endif
