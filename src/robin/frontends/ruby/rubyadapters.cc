// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/ruby/rubyadapters.cc
 *
 * @par TITLE
 * Ruby Adapter Implementations
 *
 * @par PACKAGE
 * Robin
 */

#include "rubyadapters.h"

#include <ruby.h>


namespace Robin {


void RubyLongAdapter::put(ArgumentsBuffer& argsbuf, scripting_element value)
{
  argsbuf.push(NUM2LONG(reinterpret_cast<VALUE>(value)));
}

scripting_element RubyLongAdapter::get(basic_block data)
{
  long value = reinterpret_cast<long>(data);
  return reinterpret_cast<scripting_element>(LONG2NUM(value));
}



} // end of namespace Robin


