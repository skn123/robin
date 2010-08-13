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
#include "callrequest.h"
#include "low_level.h"
#include "callable.h"
#include "robintype.h"


namespace Robin {


/**
 * @class CFunction
 * @nosubgrouping
 *
 * A single function prototype, where there is exactly
 * one set of arguments acceptable (thus, there are no competing
 * overloaded alternatives). The prototype is described through the
 * <classref>RobinType</classref> class. When the function
 * is invoked using call(), scripting-element arguments are
 * translated by querying the registered <classref>RobinType</classref>s
 * and forwarded to an approperiate low-level call.
 */
class CFunction
{
public:
    /**
     * @name Constructors
     */
	enum FunctionKind {
		GlobalFunction , Method, StaticMethod, Constructor, Destructor
	};

    //@{
    CFunction(symbol function, std::string functionName, FunctionKind functionKind, std::string className);
    CFunction(symbol function, std::string functionName, FunctionKind functionKind);
    //@}

	virtual ~CFunction();

    /**
     * @name Declaration API
	 *
	 * Methods for shaping the prototype of the C-Function
	 * in terms of the Internal Reflection data structures.
     */
    //@{
    void specifyReturnType(Handle<RobinType> type);
    void addFormalArgument(Handle<RobinType> type);
    void addFormalArgument(std::string name, Handle<RobinType> type);
    void supplyMemoryManagementHint(bool is_return_owner);

    /**
     * Determines whether edge conversions will be applied to the return
     * value of the function before passing it on to the interpreter.
     */
	bool m_allow_edge;
	//@}

	/**
	 * @name Access
	 */
	//@{
	const RobinTypes& signature() const;
	Handle<RobinType> returnType() const;
    const std::vector<std::string> &argNames() const;
    //@}


    //@{
    bool checkProperArgumentsPassed(CallRequest &req) const;
    //@}
    
    /**
     * @name Calling Conventions
     */

    //@{
	void              call(void *thisarg) const;
    basic_block       call(const ArgumentsBuffer& args) const;
    scripting_element call(const ActualArgumentList& args,
                           scripting_element owner=0) const;  
	scripting_element call(size_t nargs,
						   const ActualArgumentArray args,
						   scripting_element owner=0) const;

	//@}


protected:
	scripting_element owned(scripting_element value,
	                        scripting_element owner) const;

private:
    typedef std::map<std::string, unsigned int> ArgumentPositionMap;

    symbol                   m_functionSymbol;
    RobinTypes          m_formalArguments;
    std::vector<std::string> m_formalArgumentNames;
    Handle<RobinType>   m_returnType;
    bool                     m_returnIsOwner;
    
    ArgumentPositionMap m_formalArgumentNamePositionMap;
    std::string m_functionName;
    FunctionKind m_funcKind;
    std::string m_className;
    friend std::ostream &operator<<(std::ostream &o,const CFunction &fun);
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

