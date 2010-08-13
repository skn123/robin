#include "elements.h"


#include <iostream>


namespace Simple {

std::ostream& operator<<(std::ostream& os, const Simple::Element& e)
{
    	e.dbgout(os);
    	return os;
}

void Element::dbgout(std::ostream &out) const
{
  out << "?";
}

Element::~Element()
{
}


void Integer::dbgout(std::ostream &out) const
{
  out << value;
}


void Long::dbgout(std::ostream &out) const
{
  out << value;
}


void Float::dbgout(std::ostream &out) const
{
  out << value;
}


void String::dbgout(std::ostream &out) const
{
  out << "\"" << value << "\"";
}

void Char::dbgout(std::ostream &out) const
{
  out << "\"" << value << "\"";
}

void Object::dbgout(std::ostream &out) const
{
  out << "\"" << value << "\"";
}

} // end of namespace Simple
