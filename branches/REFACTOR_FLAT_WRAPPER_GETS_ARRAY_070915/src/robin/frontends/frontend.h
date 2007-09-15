// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/frontend.h
 *
 * @par TITLE
 * General Frontend Interface
 *
 * Defines the basic interface a frontend should live up to.
 * Frontend writes should implement the virtual methods of the interface in
 * order to introduce a new frontend.<br />
 * Since a frontend represents a language or scripting environment to support,
 * the number of expected different implementations for this interface is
 * small.
 */

#ifndef ROBIN_FRONTEND_INTERFACE_H
#define ROBIN_FRONTEND_INTERFACE_H

// STL includes
#include <exception>

// Pattern includes
#include <pattern/handle.h>

// Package includes
#include "adapter.h"
#include "../reflection/insight.h"


namespace Robin {

class TypeOfArgument;
class Library;
class LowLevel;
class Interceptor;
class ErrorHandler;

/**
 * @class Frontend
 * @nosubgrouping
 *
 * Interface class. A means for connecting between the
 * reflection abstraction layer and a targeted scripting environment.
 * <p>A frontend has to main roles:
 * <ul>
 *   <li>Passive Translation - detecting the type of an opaque
 *     scripting element received from the interpreter</li>
 *   <li>Active Translation - converting a scripting element to
 *     something which can be placed in an <classref>ArgumentsBuffer</classref>
 *     and then send to function calls. This is achieved by the
 *     frontend serving as an <classref>Adapter</classref> factory,
 *     providing functors which convert and push each of the known
 *     value types.</li>
 * </ul>
 * </p>
 * <p>Also, the frontend has two roles which are not related to translation,
 * and they are:
 * <ul>
 *   <li>Exposure - making reflection objects visible to the user
 *       interacting directly with the scripting environment</li>
 *   <li>Conversion Deployment - registering standard conversions between
 *       elements in the scripting environment, into the 
 *       <classref>ConversionTable</classref>.</li>
 * </ul>
 * </p>
 */
class Frontend
{
public:
	/**
	 * @name Deployment
	 */

	//@{

	/**
	 * This method is called when the frontend is
	 * activated. Use it to initialize global structures, and to
	 * deploy standard conversions.
	 */
	virtual
	void initialize() const = 0;

	//@}

	/**
	 * @name Passive Translation
	 *
	 * @par REFER-TO
	 * HLDD 6.6.1
	 */
	//@{

	/**
	 * Describes the type of a scripting element
	 * in terms of the internal reflection, that is, 
	 * <classref>TypeOfArgument</classref> objects. The return
	 * type can be one of the intrinsic type arguments, a class
	 * instance type, or a frontend-specialized type.
	 */
	virtual 
	Handle<TypeOfArgument> detectType(scripting_element element) const = 0;

	/**
	 * Gives additional information on a value besides its type.
	 */
	virtual Insight detectInsight(scripting_element element) const = 0;
	//@}

	/**
	 * @name Active Translation
	 *
	 * @par REFER-TO
	 * HLDD 6.6.2.1
	 */
	//@{

	/**
	 * Provides a suitable translation for converting
	 * between scripting elements of the specified type and ANSI-C
	 * calling convention formats. The frontend should implement its
	 * own set of <classref>Adapter</classref>s and return
	 * references to instances of them when requested.
	 */
	virtual
	Handle<Adapter> giveAdapterFor(const TypeOfArgument& type) const = 0;
	//@}

	/**
	 * @name Exposure
	 *
	 * @par REFER-TO
	 * HLDD 6.6.3
	 */
	//@{

	/**
	 * Creates scripting-environment suited objects
	 * and commits registration calls as required by the targeted
	 * scripting environment, in order to make a library visible
	 * to the user of that environment. This means, that the
	 * user will "see" classes and functions contained in this
	 * library by referring to them as usually accustomed in the
	 * specific language.
	 */
	virtual
	void exposeLibrary(const Library& newcomer) = 0;
	//@}

	/**
	 * @name Memory Management
	 */
	//@{

	/**
	 * Creates a new reference to an existing object.
	 * The reflection will invoke this method when the reference to an
	 * object is needed somewhere else, so that the number of references
	 * to this object increases by one. Deletion of this object should
	 * be postponed until the reference count drops back to zero.
	 */
	virtual
	scripting_element duplicateReference(scripting_element element) = 0;

	/**
	 * Frees a reference to a scripting element object.
	 * The reflection will invoke this method when it's done using an
	 * element under its supervision. The Frontend implementation should
	 * apply the approperiate garbage-collection scheme suitable for the
	 * current scripting environment.
	 */
	virtual
	void release(scripting_element element) = 0;

    /**
     * Assigns 'ownership' (control over instance deletion) for the slave to the
     * master.
     *
     * This means that as long as the 'real' reference lives, the 'imposter' lives as well.
     */
    virtual
    void bond(scripting_element master, scripting_element slave) = 0;
	//@}

	/**
	 * @name Lower Levels Interaction
	 */

	//@{
	
	/**
	 * Returns the low level interface for access to the low level function
	 * calls, matching this frontend.
	 */
	virtual
	const LowLevel& getLowLevel() const = 0;

	/**
	 * Returns an interceptor which can be used for invoking callbacks from C++
	 * interfaces which are implemented in the target scripting language.
	 */
	virtual
	const Interceptor& getInterceptor() const = 0;

	/**
	 * Returns an error handler which can be used to set and retrieve errors
	 * from the target scripting environment and from the C++ code.
	 */
	virtual
	ErrorHandler& getErrorHandler() = 0;
	//@}

};


/**
 * Allows a user to provide access to other elements in the environment,
 * thus extending the existing type-system.
 */
class UserDefinedTranslator
{
public:
	virtual
	Handle<TypeOfArgument> detectType(scripting_element element) = 0;
};



} // end of namespace Robin

#endif

