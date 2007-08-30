// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * cfunction.h
 *
 * @par TITLE
 * C-Function
 *
 * The medium-level calling convention (that is, calling with
 * no overloading or implicit conversions) is set up here using the
 * <classref>CFunction</classref> object.
 */

#ifndef ROBIN_CFUNCTION_H
#define ROBIN_CFUNCTION_H

// STL includes
#include <vector>
#include <string>
#include <set>
#include <map>

// Pattern includes
#include <pattern/handle.h>

// Package includes
#include "low_level.h"
#include "callable.h"
#include "typeofargument.h"


namespace Robin {

typedef	std::vector<Handle<TypeOfArgument> > FormalArgumentList;

/**
 * @class CFunction
 * @nosubgrouping
 *
 * A single function prototype, where there is exactly
 * one set of arguments acceptable (thus, there are no competing
 * overloaded alternatives). The prototype is described through the
 * <classref>TypeOfArgument</classref> class. When the function
 * is invoked using call(), scripting-element arguments are
 * translated by querying the registered <classref>TypeOfArgument</classref>s
 * and forwarded to an approperiate low-level call.
 */
class CFunction
{
public:
    /**
     * @name Constructors
     */

    //@{
    CFunction(symbol function);
    //@}

	virtual ~CFunction();

    /**
     * @name Declaration API
	 *
	 * Methods for shaping the prototype of the C-Function
	 * in terms of the Internal Reflection data structures.
     */

    //@{
    void specifyReturnType(Handle<TypeOfArgument> type);
    void addFormalArgument(Handle<TypeOfArgument> type);
    void addFormalArgument(std::string name, Handle<TypeOfArgument> type);

	//@}
	/**
	 * @name Access
	 */

	//@{
	const FormalArgumentList& signature() const;
	Handle<TypeOfArgument> returnType() const;
    const std::vector<std::string> &argNames() const;

    //@}

    /**
     * @name Translation of keyword arguments
     */
    
    //@{
    Handle<ActualArgumentList> mergeWithKeywordArguments(const ActualArgumentList &args, const KeywordArgumentMap &kwargs) const;

    //@}
    
    /**
     * @name Calling Conventions
     */

    //@{
	void              call() const;
	void              call(void *thisarg) const;
    basic_block       call(const ArgumentsBuffer& args) const;
    scripting_element call(const ActualArgumentList& args) const;  
	scripting_element call(size_t nargs, 
						   const ActualArgumentArray args) const;

	//@}

private:
    typedef FormalArgumentList arglist;
    typedef std::map<std::string, unsigned int> ArgumentPositionMap;

    symbol                   m_functionSymbol;
    Handle<TypeOfArgument>   m_returnType;
    arglist                  m_formalArguments;
    std::vector<std::string> m_formalArgumentNames;

    ArgumentPositionMap m_formalArgumentNamePositionMap;
};


/**
 * @class UserExceptionOccurredException
 * @nosubgrouping
 *
 * Thrown when a user function throws an exception during
 * a low-level call.
 */
class UserExceptionOccurredException : public std::exception
{
public:
	UserExceptionOccurredException();
	UserExceptionOccurredException(std::string what);

	~UserExceptionOccurredException() throw ();

    const char *what() const throw() { return "unhandled user exception."; }

	std::string user_what;
};

} // end of namespace Robin

#endif

