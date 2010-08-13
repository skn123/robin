/*
 * pointer.h
 *
 *  Created on: Feb 2, 2010
 *      Author: Marcelo Taube
 */

#ifndef ROBIN_POINTER_H_
#define ROBIN_POINTER_H_

#include "robintype.h"
#include <pattern/handle.h>
namespace Robin {

/**
 * Notice this is part of a future development.
 * It implements recursive pointer functionality.
 * It is not related to the current+stable pointer to class
 * mechanism.
 * A value of type 'int *' or 'std::vector **' will be of this
 * kind.
 *
 */
class PointerType : public RobinType {
private:
	explicit PointerType(const RobinType & pointed);
public:
	/**
	 * All these functions do the job of the constructor,
	 * but return a handle.
	 * If inheriting from this class, it is important to reimplement
	 * the method create_new completely without using the version from the parent class.
	 * @name Factory functions
	 */
	static Handle<RobinType> create_new(const RobinType &pointed);


	virtual std::string getTypeName() const;
	virtual ~PointerType();
	const RobinType& pointed() const;
protected:
	const RobinType &m_pointed;
};


} //end of namespace robin
#endif /* ROBIN_POINTER_H_ */
