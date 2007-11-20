// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/class.cc
 *
 * @par TITLE
 * Class Implement
 *
 * @par PACKAGE
 * Robin
 */

#include <assert.h>
#include <set>
#include <algorithm>

#include "class.h"
#include "instance.h"
#include "cfunction.h"
#include "overloadedset.h"
#include "namespace.h"
#include "typeofargument.h"
#include "../frontends/adapter.h"
#include "../debug/trace.h"

namespace Robin {

struct CreatedInstance
{
	void *object;
};

/**
 * Used as the return value of a function, this special
 * argument type causes it to return a 'CreatedInstance' structure,
 * holding the pointer which is expected to be returned by the
 * function, making it a creator.<p>
 * It is meant for this adapter to be assign to a TypeOfArgument which
 * denotes a pointer to the class for which this function acts as a
 * creator.</p>
 */
class CreatedInstanceAdapter : public Adapter
{
public:
	virtual void put(ArgumentsBuffer& argsbuf, scripting_element)
	{ assert(false);  /* this adapter must be used in a return type */ }

	virtual scripting_element get(basic_block data)
	{
		CreatedInstance *ci = new CreatedInstance;
		ci->object = reinterpret_cast<void*>(data);
		return ci;
	}
};






/**
 * Builds a class and assigns a name to it. Using this
 * constructor the Class is created independent of any Library.<br />
 * The <code>fullname</code> argument is expected to be unique for
 * each class, so supply a fully-qualified name (including any container
 * namespaces).
 */
Class::Class(std::string fullname)
  : m_fullname(fullname),
    m_inner(new Namespace),
	m_creators(new OverloadedSet),
	m_hasConstructors(false)
{
}

/**
 * In order for the class to be able to generate
 * instances of itself, it must hold a reference to 'this' via
 * a handle. This is both an unfortunate design flaw and an incapability
 * of C++.
 */
void Class::activate(Handle<Class> self)
{
	m_handle_to_self = self;
	// Build the TypeOfArguments needed to create instances
	// and transfer them
	m_ptrarg = Handle<TypeOfArgument>(new TypeOfArgument(self));
	m_refarg = Handle<TypeOfArgument>(new TypeOfArgument(self));
	m_outarg = Handle<TypeOfArgument>(new TypeOfArgument(self));
	m_createdarg = Handle<TypeOfArgument>(new TypeOfArgument(self));
	// The m_createdarg is internally used by the creators.
	// Assign a tailored Adapter for it.
	Handle<Adapter> created_adapter(new CreatedInstanceAdapter);
	m_createdarg->assignAdapter(created_adapter);

	dbg::trace << "// @REGISTER: " << self->name() << "* == "
			   << &*(m_ptrarg) << dbg::endl;
	dbg::trace << "// @REGISTER: " << self->name() << "& == "
			   << &*(m_refarg) << dbg::endl;
}

Class::~Class() { }



/**
 * Return's the class's unique name. The name is qualified
 * with any namespace or other class in which this class is contained.
 */
std::string Class::name() const
{
	return m_fullname;
}

/** 
 * Returns the class's contained namespace. This namespace
 * is used to hold:
 * <ul>
 *  <li>static methods</li>
 *  <li>static data members</li>
 *  <li>any inner classes</li>
 * </ul>
 * It does not, however, store constructors or inner methods.
 */
Handle<Namespace> Class::innerNamespace() const
{
	return m_inner;
}

/**
 * Finds the method with the specified name in the map
 * of instance methods. It is returned as a non-bound 
 * <classref>CallableWithInstance</classref> handle.<br />
 * NoSuchMethodException is thrown if there is no method with that
 * name in the collection.
 */
Handle<CallableWithInstance> Class::findInstanceMethod(const std::string&
													   methodname) const
{
	Handle<StandardMethod> look = lookupInstanceMethod(methodname);

	if (look) {
		return static_hcast<CallableWithInstance>(look);   // Upcast
	}
	else {
		throw NoSuchMethodException();
	}
}

/**
 * Finds the method with the specified name in the map
 * of instance methods. It is returned as a non-bound 
 * <classref>Callable</classref> handle.<br />
 * NoSuchMethodException is thrown if there is no method with that
 * name in the collection.
 */
Handle<Callable> Class::findUnboundInstanceMethod(const std::string&
													   methodname) const
{
	Handle<StandardMethod> look = lookupInstanceMethod(methodname);

	if (look) {
		return static_hcast<Callable>(look);   // Upcast
	}
	else {
		throw NoSuchMethodException();
	}
}


/**
 * Check for the existance of a specific instance method by name.
 * Returns <b>true</b> if an instance method by that name exists in the
 * class' instance method map, <b>false</b> otherwise - no exceptions are
 * thrown.
 */
bool Class::hasInstanceMethod(const std::string& methodname) const
{
	Handle<StandardMethod> look = lookupInstanceMethod(methodname);
	return look;
}

/**
 * (internal) searches a method, by name, in the instance
 * method map.<br />
 * Returns the method as a StandardMethod, which is derived from
 * <classref>OverloadedSet</classref>. A null handle is returned if
 * no such method exists.
 */
Handle<StandardMethod> Class::lookupInstanceMethod(const std::string&
												   methodname) const
{
	methodmap::const_iterator look = m_instanceMethods.find(methodname);

	if (look == m_instanceMethods.end()) {
		// Don't give up! look method up in base classes
		Handle<StandardMethod> up;
		std::vector<Handle<StandardMethod> > ups;
		
		for (std::vector<Handle<Class> >::const_iterator 
				 base_iter = m_baseClasses.begin();
			 base_iter != m_baseClasses.end(); ++base_iter) {
			// Recursively look up
			up = (*base_iter)->lookupInstanceMethod(methodname);
			if (up) ups.push_back(up);
		}
		if (ups.size() == 0)
			// Now it's time to give up
			return Handle<StandardMethod>();
		else if (ups.size() == 1)
			return ups[0];
		else {
			// Generate a new set containing *all* alternatives combined
			up = Handle<StandardMethod>(new StandardMethod);
			for (std::vector<Handle<StandardMethod> >::iterator ui = ups.begin()
					 ; ui != ups.end(); ++ui) {
				up->addAlternatives(**ui);
			}
			return up;
		}
	}
	else {
		return look->second;
	}
}

/**
 * Returns a list containing all the names of the methods
 * this class holds. Overloaded methods are grouped to one entry in
 * that list, so it contains no identical elements.
 */
std::vector<std::string> Class::listMethods() const
{
	std::vector<std::string> names;

	for (methodmap::const_iterator methoditer = m_instanceMethods.begin();
		 methoditer != m_instanceMethods.end(); ++methoditer) {
		// Add method name to list
		names.push_back(methoditer->first);
	}

	return names;
}

/**
 * Returns a list containing all the names of the methods
 * this class holds, including inherited methods. Overloaded methods
 * are grouped to one entry in that list, so it contains no identical elements.
 */
std::vector<std::string> Class::listConciseMethods() const
{
	std::vector<Handle<Class> >::const_iterator base_iter;
	std::set<std::string> collected;

	// Add base classes' method names to collected set
	for (base_iter = m_baseClasses.begin(); base_iter != m_baseClasses.end();
		 ++base_iter) {
		std::vector<std::string> parenting =
			(*base_iter)->listConciseMethods();
		std::copy(parenting.begin(), parenting.end(), 
				  std::insert_iterator<std::set<std::string> >
				  (collected, collected.begin()));
	}

	// Add my own methods to collected set
	std::vector<std::string> compensating = listMethods();
	std::copy(compensating.begin(), compensating.end(), 
			  std::insert_iterator<std::set<std::string> >
			  (collected, collected.begin()));

	// Summarize
	std::vector<std::string> summary;
	std::copy(collected.begin(), collected.end(),
			  std::insert_iterator<std::vector<std::string> >
			  (summary, summary.end()));

	return summary;
}

/**
 * Returns <b>true</b> if the class declaration is currently empty.
 * This may occur when classes are referenced but not wrapped (a kind of
 * forward declaration) or when libraries are half-way through loading.
 */
bool Class::isEmpty() const
{
	return (m_instanceMethods.size() == 0 && !m_hasConstructors);
}


/*
 * Returns a list of base classes for this class
 */
const std::vector<Handle<Class> > &Class::getBases() const
{
    return m_baseClasses;

}

/**
 * Registers a constructor. The constructor can be
 * overloaded, upon creating an instance the appropriate alternative
 * is chosen using OverloadedSet.<br />
 * Unlike in C++, no constructors are implicitly defined - so you
 * must register both the default and copy constructors manually.
 */
void Class::addConstructor(Handle<CFunction> ctorimp)
{
	ctorimp->specifyReturnType(m_createdarg);
	m_creators->addAlternative(ctorimp);
	m_hasConstructors = true;
}

/**
 * Registers an instance method. A single prototype
 * can be registered each time. When an overloaded method is
 * declared, each alternative should be registered separately, and
 * they should bare the same name - this way the Class object packs
 * them into one <classref>OverloadedSet</classref> object.
 */
void Class::addInstanceMethod(std::string methodname, 
							  Handle<CFunction> methodimp,
							  bool allow_edge)
{
	Handle<StandardMethod> existing = lookupInstanceMethod(methodname);

	if (existing) {
		// Overload
		existing->addAlternative(methodimp);
	}
	else {
		// Add new name to method list
		Handle<StandardMethod> newset(new StandardMethod);
		newset->setAllowEdgeConversions(allow_edge);
		newset->addAlternative(methodimp);
		m_instanceMethods[methodname] = newset;
	}
}

/**
 * Registers a destructor function. The destructor can have only one
 * alternative, and it must conform to the prototype:
 * <code>void deallocator(Class *self)</code>
 */
void Class::setDestructor(Handle<CFunction> dtorimp)
{
	m_deallocator = dtorimp;
}

/**
 * Used to declare inheritance. After this call, the
 * class is treated as a derived class of 'baseclass'. This means
 * that methods of 'baseclass' can be invoked on any instance of
 * this class, and that an upcast conversion is available between
 * the class and 'baseclass'.
 */
void Class::inherit(Handle<Class> baseclass)
{
	m_baseClasses.push_back(baseclass);
}



/**
 * Builds an instance of the class using the default
 * constructor. NoSuchConstructorException is thrown if there is
 * none, or if it's private.
 */
Handle<Instance> Class::createInstance() const
{
	assert(m_handle_to_self);
	if (!m_hasConstructors) throw NoConstructorsAtAllException();
	// Find the default constructor
	FormalArgumentList noargs;
	Handle<CFunction> default_ctor = m_creators->seekAlternative(noargs);
	// Call the default constructor, if it exists
	if (default_ctor) {
		ArgumentsBuffer args;
		void *new_instance =
			reinterpret_cast<void *>(default_ctor->call(args));
		Handle<Instance> hnew(new Instance(m_handle_to_self, new_instance,
										   true));
		return hnew;
	}
	else
		throw NoSuchConstructorException();
}

/**
 * Builds an instance from another one using the
 * copy constructor. It first looks for exactly such a prototype
 * (which is: <code>MyClass(const MyClass& other)</code>), throwing
 * <classref>NoSuchConstructorException</classref> if it can't find one,
 * and then invokes it with the given instance, which must be of the
 * same class type - this is asserted - and returns the result.
 */
Handle<Instance> Class::createInstance(const Instance& other) const
{
	assert(m_handle_to_self);
	if (!m_hasConstructors) throw NoConstructorsAtAllException();
	// Find the copy constructor using the expected prototype
	FormalArgumentList otherarg;
	otherarg.push_back(getRefArg());
	Handle<CFunction> copy_ctor = m_creators->seekAlternative(otherarg);
	// Call the copy constructor with 'other' instance as argument
	if (copy_ctor) {
		ArgumentsBuffer args;
		args.pushPointer(other.getObject());
		void *new_instance = 
			reinterpret_cast<void *>(copy_ctor->call(args));
		Handle<Instance> hnew(new Instance(m_handle_to_self, new_instance, 
										   true));
		return hnew;
	}
	else
		throw NoSuchConstructorException();
}

/**
 * Builds an instance using an arbitrary constructor.
 * The types of the arguments point out which of the overloaded
 * constructors will be called. An <classref>OverloadedSet</classref>
 * object is in charge of making that decision, so everything that
 * is possible for method invocations is also applicable here.<br />
 */
Handle<Instance> Class::createInstance(const ActualArgumentList& ctor_args, const KeywordArgumentMap& kwargs)
    const
{
	assert(m_handle_to_self);
	if (!m_hasConstructors) throw NoConstructorsAtAllException();
	// Call the constructor through the m_creators array.
	// The creator returns a new instance which is wrapped in a
	// 'CreatedInstance' structure.
	try {
		CreatedInstance *product = (CreatedInstance*)m_creators->call(ctor_args, kwargs);
		void *new_instance = product->object;
		Handle<Instance> hnew(new Instance(m_handle_to_self, new_instance, 
										   true));
		// Cleanup
		delete product;
		return hnew;
	}
	catch (OverloadingNoMatchException& ) {
		throw NoSuchConstructorException();
	}
}

/**
 * Uses the destructor (if present) to cause deletion of the C++ instance
 * associated with an Instance object.
 */
void Class::destroyInstance(Instance& instance) const
{
	if (m_deallocator) {
		// - build the arguments buffer directly using the pointer for
		//   optimization purposes
		m_deallocator->call(instance.getObject());
	}	
}


/**
 * Returns a <classref>TypeOfArgument</classref> which refers
 * to a pointer to an object of this class type.
 */
Handle<TypeOfArgument> Class::getPtrArg() const
{
    return m_ptrarg;
}

/**
 * Returns a <classref>TypeOfArgument</classref> which refers
 * to a reference to an object of this class type.
 */
Handle<TypeOfArgument> Class::getRefArg() const
{
    return m_refarg;
}

/**
 * Returns a <classref>TypeOfArgument</classref> which refers
 * to an output argument of this class type.
 */
Handle<TypeOfArgument> Class::getOutArg() const
{
    return m_outarg;
}



/**
 */
const char *NoSuchMethodException::what() const throw()
{
	return "the requested method does not exist.";
}

/**
 */
const char *NoSuchConstructorException::what() const throw()
{
	return "the requested constructor does not exist.";
}

/**
 */
const char *NoConstructorsAtAllException::what() const throw()
{
	return "cannot create an instance of this class.";
}



} // end of namespace Robin
