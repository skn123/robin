// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/library.h
 *
 * @par TITLE
 * Library Class
 *
 * @par PACKAGE
 * Robin
 *
 * @par PUBLIC-CLASSES
 *
 * <ul><li>Library</li></ul>
 */

#ifndef ROBIN_REFLECTION_LIBRARY_H
#define ROBIN_REFLECTION_LIBRARY_H

// STL includes
#include <string>

// Package includes
#include "namespace.h"


namespace Robin {

/**
 * @class Library
 * @nosubgrouping
 *
 * Represents a collection of classes, functions, and
 * namespaces which build a module. Collecting objects into libraries
 * enforces order upon the chaos of programming elements, as well as
 * it helps to provide name collisions between independant pieces.
 */
class Library
{
public:
	/**
	 * @name Constructors
	 */

	//@{
	Library(std::string name);

	//@}
	/**
	 * @name Access
	 */

	//@{
	const std::string& name() const;

	//@}
	/**
	 * @name Container
	 */

	//@{
	Handle<Namespace> globalNamespace() const;

private:
	std::string        m_name;
	Handle<Namespace>  m_global;
};

} // end of namespace Robin

#endif
//@}
