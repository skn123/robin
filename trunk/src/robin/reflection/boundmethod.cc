// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/boundmethod.cc
 *
 * @par PACKAGE
 * Robin
 */


#include "boundmethod.h"


namespace Robin {




BoundMethod::BoundMethod(Handle<Instance> instance, 
			 Handle<CallableWithInstance> method)
  : m_instance(instance), m_method(method)
{
}

BoundMethod::~BoundMethod()
{
}


/**
 * Forwards the call to the approperiate instance
 * method.
 */
scripting_element BoundMethod::call(const ActualArgumentList& args,
                                    const KeywordArgumentMap &kwargs) const
{
    return m_method->callUpon(*m_instance, args, kwargs);
}

} // end of namespace Robin
