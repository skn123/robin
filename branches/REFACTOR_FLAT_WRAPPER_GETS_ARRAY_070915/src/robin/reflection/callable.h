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


/**
 * @brief Basic reflection objects, manipulators, and registration methods.
 */
namespace Robin {

class Instance;

/**
 * \@TYPES
 */
typedef void *scripting_element;
typedef std::vector<scripting_element> ActualArgumentList;
typedef scripting_element ActualArgumentArray[];
typedef std::map<std::string, scripting_element> KeywordArgumentMap;

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
     */
    virtual scripting_element call(const ActualArgumentList& args, const KeywordArgumentMap &kwargs) 
      const = 0;

	virtual ~Callable() { }
};

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
     */
    virtual scripting_element callUpon(Instance& self, 
				       const ActualArgumentList& args,
                       const KeywordArgumentMap &kwargs) 
      const = 0;

	virtual ~CallableWithInstance() { }
};


/**
 * @class InvalidArgumentsException
 * @nosubgrouping
 *
 * Thrown when the number or types of the arguments in
 * a function call do not match those expected by the prototype.
 */
class InvalidArgumentsException : public std::exception
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
