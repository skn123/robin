// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/overloadedset.h
 *
 * @par TITLE
 * Overloading
 *
 * @par PACKAGE
 * Robin
 *
 * @par PUBLIC-CLASSES
 *
 * <ul><li>OverloadedSet</li></ul>
 */

#ifndef ROBIN_REFLECTION_OVERLOADEDSET_H
#define ROBIN_REFLECTION_OVERLOADEDSET_H

// Pattern includes
#include <pattern/handle.h>

// Package includes
#include "callable.h"
#include "cfunction.h"
#include "callrequest.h"

namespace Robin {







/**
 * @class OverloadedSet
 * @nosubgrouping
 *
 * Implements the concept of overloading by packing
 * several prototypes (<classref>CFunction</classref>s) together, and
 * determining - at run-time - which alternative should be invoked
 * according to the types of actual arguments.
 */
class OverloadedSet : public Callable
{
protected:

	/**
	 * @name Constructors
	 */
	//@{


	OverloadedSet(const char *name);

	/**
	 * A reference counter to 'this', it reflects how many Handles
	 * point to this object.
	 *
	 * It is used so this class can generate handlers to itself
	 *
	 * Notice1: that memory used by m_refcount does not need to be released (deleted)
	 * from this class, Handle itself will delete it when it will be needed.
	 *
	 * Notice2: a constructor of this class will initialize the value of
	 *         the reference counting to 1
	 *        and at the end of the function create_new the value has to
	 *        be decreased by one (to make sure the object will not be
	 *        released accidentally during the construction).
	 */
	mutable int *m_refcount;


public:
	/**
	 * Does the job of the constructor, but returns a handle
	 */
	static Handle<OverloadedSet> create_new(const char *name);

	/**
	 * Get a handle to this object.
	 * The handle works coordinated with the rest of the handles created here.
	 */
	Handle<OverloadedSet> get_handler();
	Handle<OverloadedSet>  get_handler() const; //some day will have to add a const handler for const correctness

	virtual ~OverloadedSet();

	//@}

	/**
	 * @name Declaration API
	 */
	//@{
	void addAlternative(Handle<CFunction> alt);
	void addAlternatives(const OverloadedSet& more);
	//@}




	/**
	 * CallResolution represents instructions to do a particular function call, including which
	 * overloaded function to call and which implicit conversion to do
	 */
	class CallResolution;


	Handle<CallResolution> resolveCall( Handle<CallRequest> &callRequest) const;


	/**
	 * @name Call
	 *
	 * Implementation of the <classref>Callable</classref>
	 * interface.
	 */
	//@{
	virtual scripting_element call(const Handle<ActualArgumentList>& args,
			const KeywordArgumentMap &kwargs, scripting_element owner=0) const;

	virtual Handle<WeightList> weight(const Handle<ActualArgumentList>& args,
			const KeywordArgumentMap &kwargs) const;

    virtual Handle<Callable> preResolveCallable() const;

	//@}

	/**
	 * @name Access
	 *
	 * Provides access to the internal metadata.
	 * For use by Robin classes only.
	 */
	//@{
	Handle<CFunction> seekAlternative(const RobinTypes& prototype)
		const;
	/**
	 * Whatever there are implementations for this method
	 */
	bool is_empty() const;

	//@}

	/**
	 * @name Internal Maintenance
	 */
	//@{
	static void forceRecompute();
	//@}


#ifdef IS_ARGUMENT_LIMIT
	static const size_t ARGUMENT_ARRAY_LIMIT;
#endif

public:
    typedef std::vector<Handle<CFunction> > altvec;
    typedef std::list<Handle<CFunction> > altlist;


	/**
	 * the name of the overloaded function
	 */
	std::string m_name;

	void printCandidates(std::ostream &o) const;
	static void printCandidates(std::ostream &o, const altlist &candidates);
	static void printCandidates(std::ostream &o, const altvec &candidates);
protected:
    bool isKwargsValid(const CFunction& cfunc, const ActualArgumentList& args,
    		const KeywordArgumentMap &kwargs) const;

private:

	altvec m_alternatives;

	class Cache;
	mutable Handle<Cache> m_cache;



#ifdef HARD_PROFILE
	class CacheReporter;
	friend class CacheReporter;
#endif
};



/**
 * @class OverloadingNoMatchException
 * @nosubgrouping
 */
class OverloadingNoMatchException : public CannotCallException
{
public:
	const char *what() const throw();
	OverloadingNoMatchException(const OverloadedSet &overloadedmethod, const Robin::CallRequest &req) throw();
	~OverloadingNoMatchException() throw();
private:
	std::string m_message;
};

/**
 * @class OverloadingAmbiguityException
 * @nosubgrouping
 */
class OverloadingAmbiguityException : public CannotCallException
{
public:
	const char *what() const throw();
	OverloadingAmbiguityException(const OverloadedSet &overloadedmethod, const OverloadedSet::altlist &competingAlternatives) throw();
	~OverloadingAmbiguityException() throw();
private:
std::string m_message;
};

/**
 * @class ArgumentArrayLimitExceededException
 * @nosubgrouping
 *
 * This exception is thrown when the number of arguments passed to a function
 * call is too great for the overloading resolution mechanism to handle.
 */
class ArgumentArrayLimitExceededException : public CannotCallException
{
public:	const char *what() const throw();
};

} // end of namespace Robin

#endif
