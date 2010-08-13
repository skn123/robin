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

//needed by the preresolving mechanism
#include "preresolveoverloadedset.h"
#include "callresolution.h"

namespace Robin {

class Instance;
class Namespace;
class CFunction;
class RobinType;

class StandardMethod : public RegularMethod<OverloadedSet>
{
public:
	/**
	 * Constructor from method name
	 */
	StandardMethod(const char *name);

	/**
	 * It returns a CachingStandardMethod of this method.
	 */
    virtual Handle<CallableWithInstance> preResolveCallableWithInstance() const;
};


/**
 * It is like a StandardMethod but it caches callsResolutions, suitable for returning
 * by preResolveCallableWithInstance methods.
 * It is implemented using PreResolveOverloadedSet
 */
class CachingStandardMethod : public RegularMethod<PreResolveOverloadedSet>
{
public:
	/**
	 * It constructs a CachingStandardMethod by selecting the method to cache.
	 */
	CachingStandardMethod(const StandardMethod *standardMethod);

	/**
	 * The copy constructor makes a new CachingStandardMethod which
	 * copies the already cached values, but can continue in a different direction.
	 */
	CachingStandardMethod(const CachingStandardMethod &cachingStandardMethod);

	/**
	 * Will return a copy of this CAchingStandardMethod
	 * using the copy constructor.
	 * In spite of that, ask yourself why would you cache an already cached method/
	 */
    virtual Handle<CallableWithInstance> preResolveCallableWithInstance() const;

};







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

    /**
     * @name Constructors
     */

    //@{
    Class(std::string fullname);


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
    //@}

public:
	/**
	 * Does the job of the constructor, but returns a handle
	 */
	static Handle<Class> create_new(std::string fullname);

    ~Class();

	/**
	 * Get a handle to this object.
	 * The handle works coordinated with the rest of the handles created here.
	 */
	Handle<Class> get_handler();
	Handle<Class>  get_handler() const; //some day will have to add a const handler for const correctness

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
    Handle<Instance> createInstance(const Handle<ActualArgumentList>& ctor_args, const KeywordArgumentMap& kwargs)
		const;

	void destroyInstance(Instance& instance) const;
    //@}

    /**
     * @name Arguments
     *
     * In order to use this class in other functions'
     * prototypes, an approperiate <classref>RobinType</classref>
     * object is supplied.
     */

    //@{
    Handle<RobinType> getPtrType() const;
    Handle<RobinType> getConstType() const;
	Handle<RobinType> getType() const;
	//@}

private:
    Handle<StandardMethod> lookupInstanceMethod(const std::string& methodname)
		const;
    Handle<StandardMethod> lookupInstanceMethodHere(const std::string& methodname)
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

    Handle<RobinType> m_ptrtype;
    Handle<RobinType> m_consttype;
    Handle<RobinType> m_type;
    Handle<RobinType> m_createdtype;


};

/**
 * @class NoSuchMethodException
 * @nosubgrouping
 *
 * Thrown when trying to invoke a method which does not
 * exist in the class.
 */
class NoSuchMethodException : public CannotCallException
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
	NoSuchConstructorException() throw();
	NoSuchConstructorException(const OverloadingNoMatchException &exp) throw();
	~NoSuchConstructorException() throw();
private:
	std::string m_message;
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

