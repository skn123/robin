/*
 * pointer.cc
 *
 *  Created on: Feb 2, 2010
 *      Author: Marcelo Taube
 */
#include "pointer.h"
namespace Robin {

PointerType::PointerType(const RobinType & pointed)
	: RobinType(TYPE_CATEGORY_POINTER,TYPE_POINTER,regularKind),
	  m_pointed(pointed)
{

}

PointerType::~PointerType()
{

}

const RobinType &PointerType::pointed() const
{
	return m_pointed;
}

std::string PointerType::getTypeName() const
{
	return m_pointed.getTypeName() + "*";
}

Handle<RobinType> PointerType::create_new(const RobinType &pointed)
{
	PointerType *self = new PointerType(pointed);
	Handle<RobinType> ret = self->get_handler();

	// m_refcount was first set to 1 to make sure the class
	// is not deleted till we return the handler, but now
	// that we have a handler we are safe
	*self->m_refcount -= 1;
	return ret;
}


} //end of namespace Robin
