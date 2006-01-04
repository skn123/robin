// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/namespace.h
 *
 * @par TITLE
 * Namespace Class
 *
 * @par PACKAGE
 * Robin
 *
 * @par PUBLIC-CLASSES
 *
 * <ul><li>Namespace</li></ul>
 */

#ifndef ROBIN_REFLECTION_NAMESPACE_H
#define ROBIN_REFLECTION_NAMESPACE_H

// STL includes
#include <string>
#include <exception>

// Pattern includes
#include <pattern/handle.h>
#include <pattern/iterator.h>


namespace Robin {

/**
 * \@TYPES
 */

class Class;
class EnumeratedType;
class Callable;

/**
 * @class Namespace
 * @nosubgrouping
 *
 * Gives a containing scope to a bunch of elements.
 * Elements which can be stored in a namespace are:
 * <ul>
 *   <li>classes</li>
 *   <li>functions</li>
 *   <li>variables (static data items)</li>
 * </ul>
 * Each of the stored elements is by nature associated with a name -
 * simply a string literal - which identifies it and serves as a
 * key via which the object is referenced.
 */
class Namespace
{
public:
	Namespace();
	~Namespace();

	/**
	 * @name Declaration API
	 */

	//@{
	void declare(std::string name, Handle<Class> element);
	void declare(std::string name, Handle<EnumeratedType> element);
	void declare(std::string name, Handle<Callable> element);
	void declare(std::string name, Handle<Namespace> element);

	void alias(std::string actual, std::string aliased);
	//@}

	/**
	 * @name Lookup
	 */

	//@{
	Handle<Class>          lookupClass    (std::string name) const;
	Handle<EnumeratedType> lookupEnum     (std::string name) const;
	Handle<Callable>       lookupFunction (std::string name) const;
	Handle<Namespace>      lookupInner    (std::string name) const;
	//@}

	/**
	 * @name Iteration
	 */
	//@{
	typedef Pattern::AbstractIterator<std::string> NameIterator;

	Handle<NameIterator> enumerateClasses() const;
	Handle<NameIterator> enumerateEnums() const;
	Handle<NameIterator> enumerateFunctions() const;
	Handle<NameIterator> enumerateAliases() const;
	//@}

private:
	struct Imp;
	Imp *imp;

	void unalias(std::string& name) const;
};

/**
 * @class LookupFailureException
 * @nosubgrouping
 *
 * Thrown when the name being given to any of the
 * Namespace::lookup... methods is not defined in the namespace.
 */
class LookupFailureException : public std::exception
{
public:
	LookupFailureException();
	LookupFailureException(std::string name);
	~LookupFailureException() throw();

	const char *what() const throw();

	std::string look;

private:
	mutable std::string msg;  /* what() buffer */
};

} // end of namespace Robin


#endif
//@}
