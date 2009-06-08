// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * method.h
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_REFLECTION_METHOD_H
#define ROBIN_REFLECTION_METHOD_H


#include "callable.h"


namespace Robin {

/**
 * @class RegularMethod
 * @nosubgrouping
 *
 * A simple template; based on a Callable class, provides
 * a trivial extension into a CallableWithInstance. Being given an
 * instance object, it prepends it to the argument list. The underlying
 * function is assumed to accept an instance object as the first
 * argument.
 */
template < class CallableBase >
class RegularMethod : public CallableBase, public CallableWithInstance
{
public:
	/**
	 * @name Constructors
	 */

	//@{
	RegularMethod();
	//@}

	virtual ~RegularMethod();

	/**
	 * @name Calling Convention
	 */

	//@{
	virtual scripting_element callUpon(Instance& self,
									   const ActualArgumentList& args, 
                                       const KeywordArgumentMap &kwargs,
                                       scripting_element owner=0) const;
	//@}
};

namespace RegularMethodInternals {
	void release(scripting_element element);
	scripting_element instanceElement(Instance& self);
}


} // end of namespace Robin

#include "method.imp"


#endif
