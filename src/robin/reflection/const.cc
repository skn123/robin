/*
 * pointer.cc
 *
 *  Created on: Feb 3, 2010
 *      Author: Marcelo Taube
 */
#include "const.h"
#include "fundamental_conversions.h"
#include "conversiontable.h"
namespace Robin {

ConstType::ConstType(const RobinType & baseType)
	: RobinType(TYPE_CATEGORY_POINTER,TYPE_POINTER,"const",constReferenceKind),
	  m_baseType(baseType)
{
	Handle<Conversion> constConversion((Conversion*)new TrivialConversion());
	constConversion->setSourceType(m_baseType.get_handler());
	constConversion->setTargetType(this->get_handler());
	// Now set weight to (1 epsilon, 0 the rest)
	constConversion->setWeight(Robin::Conversion::Weight(1,0,0,0));
	ConversionTableSingleton::getInstance()->registerConversion(constConversion);
	baseType.m_constTypeAdditionAnnouncer->notifyTypeCreated(this->get_handler());
}

ConstType::~ConstType()
{

}

const RobinType &ConstType::basetype() const
{
	return m_baseType;
}

std::string ConstType::getTypeName() const
{
	return "const " + m_baseType.getTypeName();
}

Handle<RobinType> ConstType::create_new(const RobinType &baseType)
{
	ConstType *self = new ConstType(baseType);
	Handle<RobinType> ret = self->get_handler();

	// m_refcount was first set to 1 to make sure the class
	// is not deleted till we return the handler, but now
	// that we have a handler we are safe
	*self->m_refcount -= 1;
	return ret;
}

Handle<ConstType> ConstType::searchOrCreate(const RobinType &baseType)
{

	Handle<RobinType> constType = baseType.m_constTypeAdditionAnnouncer->getTypeIfExists();
	if(constType) {
		return constType;
	}
	constType = create_new(baseType);
	return constType;
}

bool ConstType::isHyperGeneric() const
{
	return m_baseType.isHyperGeneric();
}

} //end of namespace Robin
