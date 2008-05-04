/* -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file
 *
 * @par SOURCE
 * registration/mechanism.h
 *
 * @par TITLE
 * Registration Mechanism Control Center
 *
 * @par PACKAGE
 * Robin
 *
 * @par PUBLIC-CLASSES
 *
 * <ul>
 *  <li>RegistrationMechanism</li>
 *  <li>RegistrationMechanismSingleton</li>
 * </ul>
 */

#ifndef ROBIN_REGISTRATION_MECHANISM_H
#define ROBIN_REGISTRATION_MECHANISM_H

#include <string>
#include <pattern/handle.h>
#include <pattern/singleton.h>

#include "regdata.h"
#include "../reflection/namespace.h"


namespace Robin {

class Library;
class Class;
class Namespace;
class CFunction;
class TypeOfArgument;
class EnumeratedType;
struct Signature;

/**
 * @class RegistrationMechanism
 * @nosubgrouping
 *
 * Provides an interface into the heart of the registration
 * mechanism, and welds registration data into the scripting
 * environment upon import.
 */
class RegistrationMechanism
{
public:
	/**
	 * @name Constructors
	 */

	//@{
	RegistrationMechanism();

	//@}
	/**
	 * @name Admitting
	 */

	//@{
	RegData *acquireRegData(std::string library);
	Handle<Library> import(const std::string& library);
	Handle<Library> import(const std::string& library, 
						   const std::string& name);

protected:
	RegData *acquireRegData_impl(std::string library);

	void admit(RegData *rbase, Handle<Class> klass, 
			   Namespace &container);
	void admitArguments(const RegData *rbase, Handle<CFunction> cfun,
						Namespace &container);
	void admitArguments(const RegData *rbase, Signature& signature,
						Namespace &container);
	void admitEnum(const RegData *rbase,
				   Handle<EnumeratedType> etype);
	void admitTrivialConversion(Handle<TypeOfArgument> from, 
								Handle<TypeOfArgument> to);
	void admitUserDefinedConversion(Handle<CFunction> ctor, 
									Handle<Class> klass, bool promotion);
	void admitUpCastConversion(Handle<Class> derived, Handle<Class> base,
							   void *transformsym);
	Handle<Class>          touchClass(const std::string& name,
									  Namespace &container);
	Handle<EnumeratedType> touchEnum (const std::string& name,
									  Namespace &container);
	Handle<TypeOfArgument> interpretType(const char *type, Namespace&);
	void normalizeName(RegData *reg) const;

	Robin::Namespace m_ns_common;
};


/**
 * @class RegistrationMechanismSingleton
 * @nosubgrouping
 *
 * Supplies the one and only registration mechanism
 * object.
 */
class RegistrationMechanismSingleton 
	: public Pattern::Singleton<RegistrationMechanism>
{ };

/**
 * @class DynamicLibraryOpenException
 * @nosubgrouping
 *
 * Thrown when the registration mechanism fails to
 * open a dynamic-library or shared object, or cannot read the 
 * registration information from it.
 */
class DynamicLibraryOpenException : public std::exception
{
public:
	DynamicLibraryOpenException();
	DynamicLibraryOpenException(int merr);

	~DynamicLibraryOpenException() throw() { }

	const char *what() const throw();

	int errno_at;
	std::string dlerror_at;
};

} // end of namespace Robin

#endif
//@}
