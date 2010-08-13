/*
 * preresolvegoverloadedset.h
 *
 *  Created on: Oct 7, 2009
 *      Author: Marcelo Taube
 */

#ifndef PRERESOLVEOVERLOADEDSET_H_
#define PRERESOLVEOVERLOADEDSET_H_

#include<list>
#include "../../pattern/handle.h"
#include "callable.h"
#include "overloadedset.h"
namespace Robin {

/*
 * A PreResolveOverloadedSet is an optimized copy of an
 * OverloadedSet.
 * It assumes that it will be  called with the exact same arguments
 * several times and stores the CallResolution of the different calls.
 * That means that in further calls the type conversions, the signature
 * of the called CFunction and the order of the parameters will not have
 * to be re-evaluating and time will be saved.
 * Please notice that OverloadedSet has this kind of cache already implemented
 * built-in. However, PreResolveOverloadedSet is better in many situations.
 * PreResolveOverloadedSet stores its cache in a list, assuming most of the
 * time the list will have only one element (if the function always called in
 * the same way).
 * If a user wants to call an overloaded set with different
 * parameters it is recommended to build separate PreResolveOverloadedSets
 * because the run will be faster.
 *
 */
class PreResolveOverloadedSet : public Callable
{
public:

	/**
	 * It constructs a PreResolveOverloadedSet by selecting which OverloadedSet
	 * to pre resolve.
	 */
	PreResolveOverloadedSet(const Handle<OverloadedSet> &overloadedSet);

	/**
	 * The copy constructor makes a new PreResolveOverloadedSet which
	 * copies the already resolved values, but can continue in a different direction.
	 */
	PreResolveOverloadedSet(const PreResolveOverloadedSet &other);

    virtual scripting_element
    		call(const Handle<ActualArgumentList>& args,
    		     const KeywordArgumentMap &kwargs,
    		     scripting_element owner=0) const;

    virtual Handle<WeightList> weight(const Handle<ActualArgumentList>& args,
		     const KeywordArgumentMap &kwargs) const;

	/**
	 * Present here only to implement the Callable interface.
	 * In spite of that, ask yourself why would you preresolve an already preresolved callable.
	 * Will return a copy of this PreResolveOverloadedSet
	 * using the copy constructor.
	 */
    virtual Handle<Callable> preResolveCallable() const;

private:
    Handle<OverloadedSet> m_overloadedSet;
    typedef std::list<Handle<OverloadedSet::CallResolution> > ResolutionsList;
    mutable ResolutionsList m_resolutions;

};



};//end of namespace robin


#endif /* PRERESOLVEOVERLOADEDSET_H_ */
