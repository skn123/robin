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
void SimpleInstanceObjectElement::dbgout() const
{
	try {
		Handle<Callable> print = value->bindMethod("print");
		ActualArgumentList args;
		dbg::Guard dg(dbg::trace);
		print->call(args);
	}
	catch (NoSuchMethodException& ) {
		std::cerr << "<" << value->getClass()->name() << ">";
	}
}




/**
 */
SimpleEnumeratedConstantElement::SimpleEnumeratedConstantElement
    (Handle<EnumeratedType> domain, int val)
	: value(domain, val)
{
}

} // end of namespace Robin
