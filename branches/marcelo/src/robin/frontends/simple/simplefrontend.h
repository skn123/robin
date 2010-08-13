// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/simple/simplefrontend.h
 *
 * @par TITLE
 * Simple Frontend Implementation
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_SIMPLE_FRONTEND_H
#define ROBIN_SIMPLE_FRONTEND_H

#include <robin/frontends/frontend.h>


namespace Robin {


/**
 * @class SimpleFrontend
 * @nosubgrouping
 *
 * Implements a frontend over a set of C++ object which
 * are all derived from Simple::Element.
 */
class SimpleFrontend : public Frontend
{
public:
	/**
	 * @name Constructor
	 */
	//@{
	SimpleFrontend();

	//@}
	/**
	 * @name Deployment
	 */

	//@{
	virtual
	void initialize() const;

	//@}
	/**
	 * @name Passive Translation
	 */

	//@{
	virtual 
	Handle<TypeOfArgument> detectType(scripting_element element) const;

	virtual Insight detectInsight(scripting_element) const;

	//@}
	/**
	 * @name Active Translation
	 */

	//@{
	virtual
	Handle<Adapter> giveAdapterFor(const TypeOfArgument& type) const;

	//@}

	/**
	 * @name Exposure
	 */

	//@{
	virtual
	void exposeLibrary(const Library& newcomer);
	//@}

	/**
	 * @name Memory Management
	 */
	//@{
	virtual
	scripting_element duplicateReference(scripting_element element);

	virtual
	void release(scripting_element element);

    virtual
    void own(scripting_element master, scripting_element slave);

    virtual
    void bond(scripting_element master, scripting_element slave);
	//@}
	
	/**
	 * @name Low Level
	 */
	//@{
	virtual
	const LowLevel& getLowLevel() const;

	virtual
	const Interceptor& getInterceptor() const;

	virtual
	ErrorHandler& getErrorHandler();
	//@}

	/**
	 * @name Destructor
	 */	
	//@{
	virtual ~SimpleFrontend();
	//@}

protected:
	LowLevel* m_lowLevel;
	ErrorHandler *m_errorHandler;
};

} // end of namespace Robin

#endif
