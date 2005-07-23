// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * class.h
 *
 * @par TITLE
 * Class Element
 *
 * An object-oriented look is given by binding methods
 * into classes. Here, utilities are provided for defining classes in
 * the internal reflection, and creating instances of them.
 */

#ifndef ROBIN_REFLECTION_CLASS_H
#define ROBIN_REFLECTION_CLASS_H

// STL includes
#include <map>
#include <vector>
#include <string>

// Local includes
#include <pattern/handle.h>

// Package includes
#include "callable.h"
#include "overloadedset.h"
#include "method.h"


namespace Robin {

class Instance;
class Namespace;
class CFunction;
class TypeOfArgument;

typedef RegularMethod<OverloadedSet> StandardMethod;

/**
 * @class Class
 * @nosubgrouping
 *
 * Represents a class in a C++ library. It stores a
 * complete signature of the class's methods and data members. Because
 * it stores the constructors as well, it exposes the functionality
 * of generating instances of the referred class.
 */
class Class
{
public:
    /**
     * @name Constructors
     */

    //@{
    Class(std::string fullname);
    void activate(Handle<Class> self);

    ~Class();

    //@}
    /**
     * @name Access
     */

    //@{
    std::string name() const;
    Handle<Namespace> innerNamespace() const;
    Handle<CallableWithInstance> findInstanceMethod(const std::string&
													methodname)const;
	bool hasInstanceMethod(const std::string& methodname) const;
    std::vector<std::string> listMethods() const;
    std::vector<std::string> listConciseMethods() const;

	bool isEmpty() const;
    //@}
    /**
     * @name Declaration API
     *
     * Methods through which the class's interface
     * can be declared in the Internal Reflection.
     */

    //@{
    void addConstructor(Handle<CFunction> ctorimp);
    void addInstanceMethod(std::string methodname, 
			   Handle<CFunction> methodimp);
	void setDestructor(Handle<CFunction> dtorimp);
    void inherit(Handle<Class> baseclass);

    //@}
    /**
     * @name Building and Destroying Instances
     *
     * It was said in the HLDD that the internal
     * data structures form "active" objects. And here is the
     * best example for that.
     */

    //@{
    Handle<Instance> createInstance() const;
    Handle<Instance> createInstance(const Instance& other) const;
    Handle<Instance> createInstance(const ActualArgumentList& ctor_args)
		const;

	void destroyInstance(Instance& instance) const;
    //@}

    /**
     * @name Arguments
     *
     * In order to use this class in other functions'
     * prototypes, an approperiate <classref>TypeOfArgument</classref>
     * object is supplied.
     */

    //@{
    Handle<TypeOfArgument> getPtrArg() const;
    Handle<TypeOfArgument> getRefArg() const;
	Handle<TypeOfArgument> getOutArg() const;
	//@}

private:
    Handle<StandardMethod> lookupInstanceMethod(const std::string& methodname)
		const;

protected:
    std::string       m_fullname;
    Handle<Namespace> m_inner;
	bool              m_hasConstructors;

    typedef std::map<std::string, Handle<StandardMethod> > methodmap;
    Handle<OverloadedSet> m_creators;
	Handle<CFunction> m_deallocator;
    methodmap m_instanceMethods;

    std::vector<Handle<Class> > m_baseClasses;

    Handle<TypeOfArgument> m_ptrarg;
    Handle<TypeOfArgument> m_refarg;
    Handle<TypeOfArgument> m_outarg;
    Handle<TypeOfArgument> m_createdarg;

    Handle<Class> m_handle_to_self;  // @@@ YUCK
};

/**
 * @class NoSuchMethodException
 * @nosubgrouping
 *
 * Thrown when trying to invoke a method which does not
 * exist in the class.
 */
class NoSuchMethodException : public std::exception
{
public:
    const char *what() const throw();
};

/**
 * @class NoSuchConstructorException
 * @nosubgrouping
 *
 * Thrown when trying to create an instance using a
 * constructor which does not exist.<br />
 * This only occurs when trying to activate a constructor directly,
 * rather then using the OverloadedSet mechanism - when creating
 * an instance via the default constructor or copy constructor.
 */
class NoSuchConstructorException : public NoSuchMethodException
{
public:
    const char *what() const throw();
};

/**
 * Thrown when trying to access a constructor from an abstract
 * class, which does not have any from Robin's point of view.
 */
class NoConstructorsAtAllException : public NoSuchConstructorException
{
public:
	const char *what() const throw();
};


} // end of namespace Robin

#endif

