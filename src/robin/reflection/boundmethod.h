// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/boundmethod.h
 *
 * @par TITLE
 * Bound Method Utility
 *
 * @par PACKAGE
 * Robin
 *
 * This is a utility provided for the use of frontend
 * writers - a simple way to refer to a class method and bind it with an
 * instance. Some popular scripting languages (like <i>Python</i>).
 *
 * @par PUBLIC-CLASSES
 * <ul><li>BoundMethod</li></ul>
 */

#ifndef ROBIN_REFLECTION_BOUNDMETHOD_H
#define ROBIN_REFLECTION_BOUNDMETHOD_H

// Package includes
#include "instance.h"
#include "callable.h"


namespace Robin {

/**
 * @class BoundMethod
 * @nosubgrouping
 *
 * Connects an instance method, which is represented by
 * a <classref>CallableWithInstance</classref> reference, and creates
 * an object which can directly be called upon. The arguments from
 * the <methodref>call()</methodref> are passed to the corresponding
 * <methodref>CallableWithInstance::callUpon()</methodref>
 * method, and the instance is taken from the data member which is
 * initialized at construction time.
 */
class BoundMethod : public Callable
{
public:
	/**
	 * @name Constructors
	 */
	//@{
	BoundMethod(Handle<Instance> instance, 
				Handle<CallableWithInstance> method);

	//@}

	virtual ~BoundMethod();

	/**
	 * @name Call
	 */
	//@{
	virtual scripting_element call(const ActualArgumentList& args,
                                   const KeywordArgumentMap &kwargs,
                                   scripting_element owner=0)
		const;
	//@}

private:
	Handle<Instance> m_instance;
	Handle<CallableWithInstance> m_method;
};

} // end of namespace Robin


#endif
