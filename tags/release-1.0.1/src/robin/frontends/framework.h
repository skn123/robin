// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/framework.h
 *
 * @par TITLE
 * Frontends Framework Engine
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_FRONTENDS_FRAMEWORK_H
#define ROBIN_FRONTENDS_FRAMEWORK_H

/**
 * \@INCLUDES
 */

// STL includes
#include <exception>

// Package includes
#include "frontend.h"


namespace Robin {

class TypeOfArgument;
class Class;

/**
 * @class FrontendsFramework
 * @nosubgrouping
 *
 * <p>Controls the loading and deployment of frontends.
 * One frontend is consisently held as the "active" frontend. The
 * frontend is "activated" by the calling program invoking the
 * selectFrontend() method - which initializes the frontend and
 * keeps an instance reference.</p>
 * <p>Only one frontend may be active at a given time; selecting
 * a frontend automatically disables any previously selected frontend
 * if there is one.</p>
 */
class FrontendsFramework
{
public:
	/**
	 * @name Overall Control
	 */

	//@{
	static void selectFrontend(Handle<Frontend> fe);

	static Frontend *activeFrontend();
	
	//@}
	/**
	 * @name Utility
	 */

	//@{
	static void fillAdapter(Handle<TypeOfArgument> toa);
	static void fillAdapters(Handle<Class> cls);
	//@}

private:
	static Handle<Frontend> active_frontend;

};

/**
 * @class EnvironmentVacuumException
 * @nosubgrouping
 *
 * Thrown when trying to acquire the active frontend,
 * but no such was previously set by the framework.
 */
class EnvironmentVacuumException : public std::exception
{
};

} // end of namespace Robin

#endif
