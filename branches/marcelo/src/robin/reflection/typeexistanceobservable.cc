/*
 * typeexistanceobservable.cc
 *
 *  Created on: Feb 3, 2010
 *      Author: Marcelo Taube
 */

#include "typeexistanceobservable.h"

namespace Robin {

void TypeExistanceObservable::observe(Handle<RobinTypeAddedObserver> observer)
{
	if(m_type) {
		observer->typeWasAdded(m_type);
	} else {
		m_observers.push_back(observer);
	}
}

void TypeExistanceObservable::notifyTypeCreated(Handle<RobinType> typecreated)
{
	m_type = typecreated;
	while(!m_observers.empty() ) {
		m_observers.front()->typeWasAdded(typecreated);
		m_observers.pop_front();
	}
}

Handle<RobinType> TypeExistanceObservable::getTypeIfExists()
{
	return m_type;
}


} //namespace Robin
