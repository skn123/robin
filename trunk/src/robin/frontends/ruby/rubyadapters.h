// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/ruby/rubyadapters.h
 *
 * @par TITLE
 * Ruby Adapter Implementations
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_SIMPLE_FRONTEND_ADAPTERS_H
#define ROBIN_SIMPLE_FRONTEND_ADAPTERS_H

// Package includes
#include <robin/frontends/adapter.h>
#include <robin/reflection/class.h>
#include <robin/reflection/enumeratedtype.h>


namespace Robin {


/**
 * Crude adapter - converts a T_FIXNUM to a C long value.
 */
class RubyLongAdapter : public Adapter
{
public:
    virtual void put(ArgumentsBuffer& argsbuf, scripting_element value);
    virtual scripting_element get(basic_block data);
};


} // end of namespace Robin



#endif
