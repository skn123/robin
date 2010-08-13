// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/namespace.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "namespace.h"
#include "callable.h"
#include "class.h"
#include "enumeratedtype.h"

#include <map>
#include <robin/debug/assert.h>


namespace Robin {


struct Namespace::Imp
{
	typedef std::map<std::string, Handle<Class> > classmap;
	typedef std::map<std::string, Handle<EnumeratedType> > enummap;
	typedef std::map<std::string, Handle<Callable> > functionmap;
	typedef std::map<std::string, Handle<Namespace> > subnamespacemap;
	typedef std::map<std::string, std::string> aliasmap;

	classmap        m_classes;     /* contained classes */
	enummap         m_enums;       /* contained enumerated types */
	functionmap     m_functions;   /* contained routines */
	subnamespacemap m_inner;       /* contained namespaces */
	aliasmap        m_aliases;     /* object aliases */
};



Namespace::Namespace(const std::string &name) : imp(new Imp), m_name(name) { }
Namespace::~Namespace() { delete imp;}



/**
 * Adds a class to the current namespace. If a class
 * with this name is already declared in the namespace, the previous
 * declaration is discarded.
 */
void Namespace::declare(std::string name, Handle<Class> element)
{
	imp->m_classes[name] = element;
}

/**
 * Adds an enumerated type to the current namespace. If
 * a type with this name is already declared in the namespace, the
 * previous declaration is discarded.
 */
void Namespace::declare(std::string name, Handle<EnumeratedType> element)
{
	imp->m_enums[name] = element;
}

/**
 * Adds a function to the current namespace. If a function
 * with this name is already declared in the namespace, the previous
 * declaration is discarded.
 */
void Namespace::declare(std::string name, Handle<Callable> element)
{
	imp->m_functions[name] = element;
}

/**
 * Nests another namespace in the current one. If a
 * namespace with this name is already declared here, the previous
 * declaration is discarded.
 */
void Namespace::declare(std::string name, Handle<Namespace> element)
{
}

/**
 * Creates a symbolic alias to another object in THIS
 * namespace. When asking for an object named 'aliased' using any of
 * the lookup methods, an object named 'actual' is looked up.
 */
void Namespace::alias(std::string actual, std::string aliased)
{
	imp->m_aliases[aliased] = actual;
}

/**
 * Removes aliasing from names, that is, returns a name
 * referring to the actual object.
 */
void Namespace::unalias(std::string& name) const
{
	Imp::aliasmap::const_iterator lui = imp->m_aliases.find(name);

	while (lui != imp->m_aliases.end()) {
		name = lui->second;
		dbg::traceClassRegistration << "was aliased to class " << name << dbg::endl;
		lui = imp->m_aliases.find( name );
	}
}


/**
 * Searches for a class contained in this namespace.
 *
 * @throws LookupFailureException if the class is not found.
 */
Handle<Class> Namespace::lookupClass(std::string name) const
{
	dbg::traceClassRegistration << "looking up class " << name << " in namespace '" << m_name << "'" << dbg::endl;
	unalias(name);
	Imp::classmap::const_iterator lui = imp->m_classes.find(name);

	if (lui == imp->m_classes.end())
		throw LookupFailureException(name);
	else {
		return lui->second;
	}
}

/**
 * Searches for an enumerated type contained in this
 * namespace.
 *
 * @throws LookupFailureException if the class is not found.
 */
Handle<EnumeratedType> Namespace::lookupEnum(std::string name) const
{
	unalias(name);
	Imp::enummap::const_iterator lui = imp->m_enums.find(name);

	if (lui == imp->m_enums.end())
		throw LookupFailureException(name);
	else
		return lui->second;
}

/**
 * Searches for a function contained in this namespace
 * (a function is an object of a type implementing the
 * <classref>Callable</classref> interface).
 *
 * @throws LookupFailureException if the routine is not found.
 */
Handle<Callable> Namespace::lookupFunction(std::string name) const
{
	unalias(name);
	Imp::functionmap::const_iterator lui = imp->m_functions.find(name);

	if (lui == imp->m_functions.end())
		throw LookupFailureException(name);
	else
		return lui->second;
}

/**
 * Searches for an inner namespace contained in this namespace.
 *
 * @throws LookupFailureException if no such inner namespace is found.
 */
Handle<Namespace> Namespace::lookupInner(std::string name) const
{
	assert_true(false); //Still not implemented!
	return Handle<Namespace>();
}

/**
 * Returns an iterator over all the classes in this namespace.
 */
Handle<Namespace::NameIterator> Namespace::enumerateClasses() const
{
	return Handle<NameIterator>(new
	  Pattern::MapKeyIterator<std::string,Imp::classmap>(imp->m_classes));
}

/**
 * Returns an iterator over all the enumerated types in this namespace.
 */
Handle<Namespace::NameIterator> Namespace::enumerateEnums() const
{
	return Handle<NameIterator>(new
	  Pattern::MapKeyIterator<std::string,Imp::enummap>(imp->m_enums));
}

/**
 * Returns an iterator over all the functions in this namespace.
 */
Handle<Namespace::NameIterator> Namespace::enumerateFunctions() const
{
	return Handle<NameIterator>(new
	  Pattern::MapKeyIterator<std::string,Imp::functionmap>(imp->m_functions));
}

/**
 * Returns an iterator over all the name aliases in this namespace.
 */
Handle<Namespace::NameIterator> Namespace::enumerateAliases() const
{
	return Handle<NameIterator>(new
		Pattern::MapKeyIterator<std::string,Imp::aliasmap>(imp->m_aliases));
}



/**
 */
LookupFailureException::LookupFailureException()
	: look("")
{ }

/**
 */
LookupFailureException::LookupFailureException(std::string name)
	: look(name)
{ }

/**
 */
LookupFailureException::~LookupFailureException() throw()
{ }

/**
 */
const char *LookupFailureException::what() const throw()
{
	if (look.size() == 0)
		return "name lookup failed.";
	else {
		msg = "name lookup failed (" + look + ").";
		return msg.c_str();
	}
}


std::ostream &operator<<(std::ostream &out, const Namespace &nameSpace) {
	out << "Namespace with " << nameSpace.imp->m_classes.size()
	<< " classes and " <<  nameSpace.imp->m_aliases.size()
	<< " aliases.";
	return out;
}

} // end of namespace Robin
