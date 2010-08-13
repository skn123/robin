// -*- C++ -*-

/**
 * @file
 *
 * @par SOURCE
 * callable.h
 *
 * @par TITLE
 * Callable interface
 *
 * @par PUBLIC-CLASSES
 *
 * <ul><li>Callable</li>
 *     <li>CallableWithInstance</li>
 * </ul>
 */

#ifndef ROBIN_CALLABLE_H
#define ROBIN_CALLABLE_H

#include <vector>
#include <map>
#include <string>
#include <exception>
#include <set>
#include "../../pattern/handle.h"
#include "types.h"
#include "conversion.h"
/**
 * @brief Basic reflection objects, manipulators, and registration methods.
 */
namespace Robin {

class Instance;



#define NONE ((void*)0)


/**
 * @class Callable
 * @nosubgrouping
 *
 * Interface for a "thingy" which can be invoked.
 */
class Callable
{
public:
    /**
     * Invokes the Callable object given a list
     * of arguments. Override to implement a callable action.
     * @param self The 'this' argument
     * @param args Arguments passed by position
     * @param kwargs Arguments passed by name
     * @param owner It means the owner of the memory returned by the function.
     * 				The object returned by this func might have pointers referencing
     * 				memory of 'owner'. Thus owner should not be released till the returned
     * 				object is released.
     * 				An example of this would be an object returning the value of a field by reference.
     * 				In those cases the reference count of owner will be increased.
     * 				NOTICE: this construct should be reviewed, the function itself should be the one
     * 				to know who the memory belongs to and increase the refcount in the proper place.
     * 				The caller should not be responsible for this.
     */
    virtual scripting_element 
    		call(const Handle<ActualArgumentList>& args,
    		     const KeywordArgumentMap &kwargs, 
    		     scripting_element owner=0) const = 0;


    virtual Handle<Callable>			preResolveCallable() const = 0;

    virtual Handle<WeightList> weight(const Handle<ActualArgumentList>& args,
		     const KeywordArgumentMap &kwargs) const = 0;

	virtual ~Callable()= 0;
};


inline Callable::~Callable() {

}

/**
 * @class CallableWithInstance
 * @nosubgrouping
 *
 * Same as <classref>Callable</classref>, except that it
 * is called upon a specific instance in addition to the list of
 * arguments. This is the superset of instance member functions.
 */
class CallableWithInstance
{
public:
    /**
     * Invokes the object with a class instance.
     * Override to implement a callable action.
     * @param self The 'this' argument
     * @param args Arguments passed by position
     * @param kwargs Arguments passed by name
     * @param owner It means the owner of the memory returned by the function.
     * 				The object returned by this func might have pointers referencing
     * 				memory of 'owner'. Thus owner should not be released till the returned
     * 				object is released.
     * 				An example of this would be an object returning the value of a field by reference.
     * 				In those cases the reference count of owner will be increased.
     * 				NOTICE: this construct should be reviewed, the function itself should be the one
     * 				to know who the memory belongs to and increase the refcount in the proper place.
     * 				The caller should not be responsible for this.
     */
    virtual scripting_element
    		callUpon(scripting_element self,
			         const ActualArgumentList& args,
			         const KeywordArgumentMap &kwargs,
			         scripting_element owner=0) 
      const = 0;


    virtual Handle<WeightList> weightUpon(scripting_element self,
										const Handle<ActualArgumentList>& args,
										const KeywordArgumentMap &kwargs) const = 0;

    /**
     * It returns a CallableWithInstance that is equal to this one
     * but it works faster if called several times with the same
     * parameters.
     * It caches data about the call resolution locally.
     */
     virtual Handle<CallableWithInstance>
			preResolveCallableWithInstance() const = 0;




	virtual ~CallableWithInstance() { }
};


/**
 * @class InvalidArgumentsException
 * @nosubgrouping
 *
 * Thrown when the number or types of the arguments in
 * a function call do not match those expected by the prototype.
 */
class InvalidArgumentsException : public CannotCallException
{
public:
    InvalidArgumentsException() : m_reason("invalid arguments."){ }
    InvalidArgumentsException(const std::string &reason) : m_reason(reason) { }
    ~InvalidArgumentsException() throw() { }
    const char *what() const throw() { return m_reason.c_str(); }
private:
    std::string m_reason;
};


} // end of namespace Robin

#endif
