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
 * RegularMethod is a CallableWithInstance based on a underlying Callable
 * object. To perform the call with an instance object, it prepends it to the argument list.
 * The underlying function is assumed to accept an instance object as the first
 * argument.
 * It is templated because it can extend any callable, the class CallableType is
 * supposed to inherit from Callable.
 *
 */
template < class CallableType >
class RegularMethod :  public CallableWithInstance
{
public:
	/**
	 * The underlying callable object
	 */
	Handle<CallableType> m_callable;

	/**
	 * @name Constructors
	 */

	//@{

		inline RegularMethod(const Handle<CallableType> &callable);
	//@}

	virtual ~RegularMethod();

	/**
	 * @name Calling Convention
	 */

	//@{
	virtual scripting_element callUpon(scripting_element myself,
									   const ActualArgumentList& args,
                                       const KeywordArgumentMap &kwargs,
                                       scripting_element owner=0) const;

    virtual Handle<WeightList> weightUpon(scripting_element self,
										const Handle<ActualArgumentList>& args,
										const KeywordArgumentMap &kwargs) const;

	//@}
};


} // end of namespace Robin

#include "method.imp"


#endif
