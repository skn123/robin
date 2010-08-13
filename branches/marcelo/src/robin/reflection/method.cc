// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * method.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "class.h"
#include "instance.h"
#include "method.h"
#include "memorymanager.h"
#include "../frontends/framework.h"
#include "../frontends/frontend.h"


namespace Robin {


namespace RegularMethodInternals {

/**
 * (internal) Creates a scripting element which wraps a
 * standard <classref>Instance</classref>. This is done using this
 * instance's class' <i>refArg</i>.
 */
scripting_element instanceElement(Instance& i) 
{
	return i.scriptify(Instance::BORROWER);
}

/**
 * (internal) Frees an associated scripting element by invoking the 
 * approperiate method of the currently active frontend.
 */
void release(scripting_element element)
{
	MemoryManager::release(element);
}


} // end of namespace RegularMethodInternals


} // end of namespace Robin
