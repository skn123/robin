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
public:
	/**
	 * @name Constructors
	 */
	//@{
	OverloadedSet();

	virtual ~OverloadedSet();

	//@}

	/**
	 * @name Declaration API
	 */
	//@{
	void addAlternative(Handle<CFunction> alt);
	void addAlternatives(const OverloadedSet& more);
	void setAllowEdgeConversions(bool allow);
	//@}

	/**
	 * @name Call
	 *
	 * Implementation of the <classref>Callable</classref>
	 * interface.
	 */
	//@{
	virtual scripting_element call(const ActualArgumentList& args, const KeywordArgumentMap &kwargs) const;

	//@}

	/**
	 * @name Access
	 *
	 * Provides access to the internal metadata.
	 * For use by Robin classes only.
	 */
	//@{
	Handle<CFunction> seekAlternative(const FormalArgumentList& prototype) 
		const;

	//@}

	/**
	 * @name Internal Maintenance
	 */
	//@{
	static void forceRecompute();
	//@}
    


private:

    typedef std::vector<Handle<CFunction> > altvec;


/*    scripting_element callWithoutKWargs(const ActualArgumentList& args) const;
    scripting_element callWithKWargs(const ActualArgumentList& args, const KeywordArgumentMap& kwargs) const;*/
    scripting_element call_impl(const ActualArgumentList& args, const KeywordArgumentMap& kwargs) const;



	altvec m_alternatives;
	bool m_allow_edge;

	class Cache;
	class CacheSingleton;

	static const int ARGUMENT_ARRAY_LIMIT;

#ifdef HARD_PROFILE
	class CacheReporter;
	friend class CacheReporter;
#endif
};


/**
 * @class OverloadingNoMatchException
 * @nosubgrouping
 */
class OverloadingNoMatchException : public std::exception
{
public:	const char *what() const throw();
};

/**
 * @class OverloadingAmbiguityException
 * @nosubgrouping
 */
class OverloadingAmbiguityException : public std::exception
{
public: const char *what() const throw();
};

/**
 * @class ArgumentArrayLimitExceededException
 * @nosubgrouping
 *
 * This exception is thrown when the number of arguments passed to a function
 * call is too great for the overloading resolution mechanism to handle.
 */
class ArgumentArrayLimitExceededException
{
public:	const char *what() const throw();
};

} // end of namespace Robin

#endif
