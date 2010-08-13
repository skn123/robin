/*
 * typeexistanceobservable.h
 *
 *  Created on: Feb 3, 2010
 *      Author: Marcelo Taube
 */

#ifndef ROBIN_TYPEEXISTANCEOBSERVABLE_H_
#define ROBIN_TYPEEXISTANCEOBSERVABLE_H_

#include "robintype.h"
#include <list>
namespace Robin {
/**
 * This is a kind of observer (callback) class which wants to
 * be notified that a new robinType was added.
 * A class which implements this interface should register itself
 * with some mechanism that will notify it.
 */
class RobinTypeAddedObserver {
public:
	virtual void typeWasAdded(Handle<RobinType> robinType) =0;
	virtual ~RobinTypeAddedObserver() = 0;
};

typedef std::list<Handle<RobinTypeAddedObserver> > RobinTypeAddedObservers;


/**
 * It is a mechanism which enables to search lazily for a type which
 * meets a specific condition.
 * It uses the Observable-Observer design pattern.
 * If the type does not exist yet:
 *    -> Instead of the type being created by the request or an error being reported
 * the mechanism returns immediately; when the type will be created later then
 * it will report to all the observers.
 * If the type exists at the time the observation was requested then it will be reported
 * immediately.
 */
class TypeExistanceObservable {
public:
	/**
	 * Request to be notified if/when the RobinType related to
	 * this TypeExistanceObservable is created.
	 */
	void observe(Handle<RobinTypeAddedObserver> observer);

	/**
	 * The RobinType related to this TypeExistanceObservable was
	 * finally created, all the observables will be updated.
	 */
	void notifyTypeCreated(Handle<RobinType> typecreated);

	/**
	 * Instant query.
	 * Returns the type being observer if it exists otherwise returns
	 * NULL.
	 */
	Handle<RobinType> getTypeIfExists();
protected:
	Handle<RobinType> m_type;
	RobinTypeAddedObservers m_observers;
};

} //end of namespace robin
#endif /* ROBIN_TYPEEXISTANCEOBSERVABLE_H_ */
