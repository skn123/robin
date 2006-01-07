#include "elements.h"


#include <iostream>


namespace Simple {


void Element::dbgout() const
{
  std::cerr << "?";
}

Element::~Element()
{
}


void Integer::dbgout() const
{
  std::cerr << value;
}


void Long::dbgout() const
{
  std::cerr << value;
}


void Float::dbgout() const
{
  std::cerr << value;
}


void String::dbgout() const
{
  std::cerr << "\"" << value << "\"";
}

} // end of namespace Simple
