// -*- mode: C++; indent-tabs-mode: nil -*-

/**
 * @file
 *
 * @par SOURCE
 * instance.h
 *
 * @par TITLE
 * Instance Object
 *
 * @par PACKAGE
 * Robin
 *
 * Encapsulates references to actual C++ object received from
 * the wrapped functions, methods or constructors and are accessible from
 * any scripting environment.
 */

#ifndef ROBIN_REFLECTION_INSTANCE_H
#define ROBIN_REFLECTION_INSTANCE_H

// Local includes
#include <pattern/handle.h>

// Package includes
#include "class.h"


namespace Robin {

/**
 * @class Instance
 * @nosubgrouping
 *
 * Holds a reference to a C++ object and the class type
 * of it in the reflection database. It allows several basic 
 * operations - most of the functionlty is concentrated in the Class
 * construct.
 */
class Instance
{
public:
    // - these enums are used for syntax sugaring of Instance::scriptify
    enum Owner { OWNER };
    enum Borrower { BORROWER };

    /**
     * @name Constructors
     */

    //@{
    Instance();
    Instance(Handle<Class> classtype, void *cppinstance, 
             bool autodestruct = false);
    //@}

    /**
     * @name Destructor
     */
    //@{
    virtual ~Instance();
    //@}

    /**
     * @name Access
     */

    //@{
    void *getObject() const;
    Handle<Class> getClass() const;

    operator void *() const;

    //@}
    /**
     * @name Utilities
     *
     * Some shortcuts to make the frontend writer's
     * life easier.
     */

    //@{
    Handle<Instance> clone() const;
    Handle<Callable> bindMethod(std::string methodname) const;

    scripting_element scriptify(Owner);
    scripting_element scriptify(Borrower);

    void own();
    void disown();
    void destroy();
    void bond(Handle<Instance> james);
    //@}

private:
    void *m_cppinstance;      // the instance in C++
    Handle<Class> m_class;    // the class of which this is an instance
    bool m_autodestruct;      // should the instance destroy the C++ instance
                              // when deleted
    Handle<Instance> m_bond;  // the bonding instance that is alive
};



} // end of namespace Robin


#endif

