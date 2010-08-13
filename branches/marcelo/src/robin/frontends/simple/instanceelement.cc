// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

#include "instanceelement.h"

#include <robin/reflection/callable.h>
#include <robin/reflection/class.h>

#include <iostream>
#include <robin/debug/trace.h>


namespace Robin {



/**
 * @file
 *
 */
SimpleInstanceObjectElement::SimpleInstanceObjectElement()
{
}

/**
 * Creates an element which is already initialized with
 * an instance.
 */
SimpleInstanceObjectElement::SimpleInstanceObjectElement(Handle<Instance>
														 instance)
	: value(instance)
{
}

/**
 * Uses the 'print' method of the instance (if there is
 * such) to provide a concise debug print.
 */
void SimpleInstanceObjectElement::dbgout(std::ostream &out) const
{
	Handle<CallableWithInstance> print =
			value->getClass()->findInstanceMethod("print");
	if(print)
	{
		ActualArgumentList args;
		dbg::Guard dg(dbg::trace);
		KeywordArgumentMap kwargs;
		/*
		 * C++ has the concept of constness but most scripting classes do not.
		 * Because of that robin does not implement calling functions with const
		 * parameters.
		 * dbgout is a const function, it does not modifies the element, only
		 * prints its representation, but it is implemented through a robin
		 * function call which means a const cast needs to be done.
		 */
		print->callUpon(const_cast<SimpleInstanceObjectElement *>(this),args,kwargs);
	} else {
		out << "<" << value->getClass()->name() << ">";
	}
}

/**
 * Creates an element which is initialized with an
 * address.
 */
SimpleAddressElement::SimpleAddressElement(Handle<Address> address)
	: value(address)
{
}

/**
 * stub
 */
void SimpleAddressElement::dbgout(std::ostream &out) const
{
	out << "<address>";
}


/**
 */
SimpleEnumeratedConstantElement::SimpleEnumeratedConstantElement
    (Handle<EnumeratedType> domain, int val)
	: value(domain, val)
{
}

} // end of namespace Robin
