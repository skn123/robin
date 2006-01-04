// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/instance.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "instance.h"

#include <assert.h>
#include "boundmethod.h"


namespace Robin {



/**
 * Empty constructor - creates a NULL instance.
 */
Instance::Instance()
{
}

/**
 * Builds an object which wraps a C++ instance object.
 * The object is referred to as an opaque pointer (void*), and, by
 * identifying the class type as well using a <classref>Class</classref>
 * handle, it receives the meaning of an object of this class type.
 *
 * @note The caller is obliged to give a pointer to an object which
 * is really of the mentioned class type. In most cases, it is
 * technically impossible to validate this at run-time.
 */
Instance::Instance(Handle<Class> classtype, void *cppinstance,
				   bool autodestruct)
	: m_cppinstance(cppinstance), m_class(classtype), 
	  m_autodestruct(autodestruct)
{
}

/**
 * Instance destructor - destroys underlying C++ object if required.
 */
Instance::~Instance()
{
	if (m_autodestruct) destroy();
}

/**
 * Returns the encapsulated C++ object.
 */
void *Instance::getObject() const
{
    return m_cppinstance;
}

/**
 * Returns the class of which the encapsulated object
 * is an instance.
 */
Handle<Class> Instance::getClass() const
{
	assert(m_class);
    return m_class;
}



/**
 * Copies the contents of the encapsulated instance
 * object using the approperiate copy constructor, and returns a new
 * Instance holding it.
 *
 * @throws OverloadingMismatchException if there is no copy
 * constructor for the class.
 */
Handle<Instance> Instance::clone() const
{
	return m_class->createInstance(*this);
}

/**
 * Finds a method routine in the class, binds it with
 * the current instance and returns a <classref>BoundMethod</classref>
 * object which implements <classref>Callable</classref>.
 *
 * @throws LookupFailureException if there is no method with the
 * given name.
 */
Handle<Callable> Instance::bindMethod(std::string methodname) const
{
	// Find the callable object in the class specifier
	Handle<CallableWithInstance> methcore = 
		m_class->findInstanceMethod(methodname);

	// Create a bound method object.
	// Note that a bound method requires a handle to the instance
	// object. This is impossible because plain handles cannot
	// provide a handle to 'this' without creating reference cycles.
	// So the instance is duplicated.
	Handle<Instance> duplica(new Instance(*this));
	duplica->m_autodestruct = false;

	return Handle<Callable>(new BoundMethod(duplica, methcore));
}

/**
 * Turns this instance into a common type for use in
 * the scripting environment.
 * Ownership is <b>stolen</b> from the original instance object and the
 * newly created scripting element <b>now owns</b> the C++ instance.
 */
scripting_element Instance::scriptify(Owner)
{
	m_autodestruct = false;
	// Utilize the class' pointer argument adapter
	return m_class->getPtrArg()->get((basic_block)m_cppinstance);
}

/**
 * Turns this instance into a common type for use in
 * the scripting environment.
 * The new object <b>"borrows"</b> a reference to the C++ instance, so 
 * ownership resposibilities are not changed as a result of this act, but
 * the caller must guarantee that this new object is not used after the
 * original object has been deleted.
 */
scripting_element Instance::scriptify(Borrower)
{
	// Utilize the class' reference argument adapter
	return m_class->getRefArg()->get((basic_block)m_cppinstance);
}

/**
 * Seizes ownership over the C++ instance held in this object - so that when
 * the reference is destroyed the object itself (m_cppinstance) is deleted.
 */
void Instance::own()
{
	m_autodestruct = true;
}

/**
 * Ceases ownership over the C++ instance held in this object - so that when
 * the reference is destroyed the object itself (m_cppinstance) is not deleted.
 */
void Instance::disown()
{
	m_autodestruct = false;
}

/**
 * Invokes the class' destructor and deletes the object held by this
 * instance.
 */
void Instance::destroy()
{
	m_class->destroyInstance(*this);
}

/**
 * Directly assigns the C++ instance held in an Instance
 * to a native pointer. This operator is only introduced as "syntax sugaring"
 * and for use in testing.
 */
Instance::operator void * () const
{
    return getObject();
}


} // end of namespace Robin

