/*
 * pointer.h
 *
 *  Created on: Feb 3, 2010
 *      Author: Marcelo Taube
 */

#ifndef ROBIN_CONST_H_
#define ROBIN_CONST_H_

#include "robintype.h"
#include <pattern/handle.h>
namespace Robin {

/**
 * Notice this is part of an ongoing development.
 * It implements recursive const functionality.
 * It is not currently related to the implementation of the
 * const reference type mechanism of the class objects (which
 * is obtained by Class::getRefArg).
 *
 * Values of type 'const PyLists &' are currently of this kind.
 * In the future values of type 'const int &' or 'const std::vector &'
 * will also be of this kind, but this is currently not supported.
 *
 */
class ConstType : public RobinType {
private:
	explicit ConstType(const RobinType & baseType);

	/**
	 * All these functions do the job of the constructor,
	 * but return a handle.
	 * If inheriting from this class, it is important to reimplement
	 * the method create_new completely without using the version from the parent class.
	 * @param baseType the base type which this type modifies
	 */
	static Handle<RobinType> create_new(const RobinType &baseType);

	/**
	 * A const type is hyper generic depending on its internal
	 * base type.
	 */
	virtual bool isHyperGeneric() const;
public:

	/**
	 * Searches for the ConstType of a base type or creates a new one
	 */
	static Handle<ConstType> searchOrCreate(const RobinType &baseType);

	virtual std::string getTypeName() const;
	virtual ~ConstType();
	const RobinType& basetype() const;
protected:
	const RobinType &m_baseType;
};


} //end of namespace robin
#endif /* ROBIN_POINTER_H_ */
