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
 */

#include "library.h"


namespace Robin {





/**
 * Creates a library and assigns a name to it.
 */
Library::Library(std::string name)
	: m_name(name), m_global(new Namespace(name))
{
}



/**
 * Returns the name of the library. A library may have
 * been created without a name; in this case, its name is set to
 * "<>".
 */
const std::string& Library::name() const
{
	return m_name;
}



/**
 * Provides access to the library's components through
 * a namespace.
 */
Handle<Namespace> Library::globalNamespace() const
{
	return m_global;
}


} // end of namespace Robin
