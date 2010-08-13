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

#include <robin/debug/assert.h>
#include <stdexcept>

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
    // if we are autodestructing, and we do not have a bonded instance - destroy
	if (m_autodestruct && !m_bond) destroy();
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
	assert_true(m_class);
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
 * Turns this instance into a common type for use in
 * the scripting environment.
 *
 * Notice that this method always creates a new scripting_element, thus
 * assuming that it was never called before. There is no sense that
 * two scripting elements represent the same Instance object, because
 * they will be treated as different in the scripting environment
 * ( 'a==b' will return false).
 *
 * Ownership is <b>stolen</b> from the original instance object and the
 * newly created scripting element <b>now owns</b> the C++ instance.
 */
scripting_element Instance::scriptify()
{
	m_autodestruct = false;
	// Utilize the class' pointer argument adapter
	return m_class->getPtrType()->get((basic_block)m_cppinstance);
}

/**
 * Seizes ownership over the C++ instance held in this object - so that when
 * the reference is destroyed the object itself (m_cppinstance) is deleted.
 */
void Instance::own()
{
	m_autodestruct = true;
    if (m_bond) {
        m_bond->own();
    }
}

/**
 * Ceases ownership over the C++ instance held in this object - so that when
 * the reference is destroyed the object itself (m_cppinstance) is not deleted.
 */
void Instance::disown()
{
	m_autodestruct = false;

    if (m_bond) {
        m_bond->disown();
    }
}

/**
 * Invokes the class' destructor and deletes the object held by this
 * instance.
 */
void Instance::destroy()
{
	m_class->destroyInstance(*this);
}

void Instance::bond(Handle<Instance> james)
{
    m_bond = james;
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

