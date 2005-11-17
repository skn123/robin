// -*- C++ -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/cfunction.cc
 *
 * @par PACKAGE
 * Robin
 *
 * @par TITLE
 * C-Function Implementation
 */

#include "cfunction.h"

#include "../debug/trace.h"
#include <robin/frontends/framework.h>

namespace Robin {





/**
 * Low-level constructor, builds function directly with
 * an address of a native C function (rather than resolving the
 * symbol from the Library).
 */
CFunction::CFunction(symbol cfun)
    : m_functionSymbol(cfun)
{
}

CFunction::~CFunction()
{
}


/**
 * Sets the expected type of the value returned from
 * the function, as a <classref>TypeOfArgument</classref> object.
 */
void CFunction::specifyReturnType(Handle<TypeOfArgument> type)
{
    m_returnType = type;
}

/**
 * Declares a formal argument to the function. The
 * declaration is appended to the already registered argument list;
 * so consequent calls to addFormalArgument build the entire 
 * prototype. Note that the order is meaningful.
 *
 * @note You can provide a name to the argument, however, it is
 * currently ignored.
 */
void CFunction::addFormalArgument(std::string name, 
				  Handle<TypeOfArgument> type)
{
    m_formalArguments.push_back(type);
    m_formalArgumentNames.push_back(name);
}

/**
 * Declares an anonymous argument (equivalent to
 * <code>addFormalArgument("", type)</code>.
 */
void CFunction::addFormalArgument(Handle<TypeOfArgument> type)
{
    addFormalArgument("", type);
}



/**
 * Return a list of this function's formal arguments.
 */
const FormalArgumentList& CFunction::signature() const
{
    return m_formalArguments;
}

/**
 * Returns the type object identified with the function's return type.
 */
Handle<TypeOfArgument> CFunction::returnType() const
{
	return m_returnType;
}

/**
 * Directly calls underlying C function with no arguments.
 * The return type is assumed void, and no exceptions are caught.
 * Use only when performance is critical.
 */
void CFunction::call() const
{
	((void(*)())m_functionSymbol)();
}

/**
 * Directly calls underlying C function with a single pointer argument.
 * The return type is assumed void, and no exceptions are caught.
 * Use only when performance is critical.
 */
void CFunction::call(void *thisarg) const
{
	((void(*)(void*))m_functionSymbol)(thisarg);
}

/**
 * Yields an approperiate low-level call with the
 * arguments in the buffer. The format of the arguments pushed to
 * the buffer is assumed to be compatible with the function's
 * prototype (this cannot be validated at runtime).
 */
basic_block CFunction::call(const ArgumentsBuffer& args) const
{
	const LowLevel *lowlevel;
	try {
		lowlevel = &(FrontendsFramework::activeFrontend()->getLowLevel());
	}
	catch (EnvironmentVacuumException &e)
	{
		lowlevel = new LowLevel();
	}
	
    try {
		lowlevel->call_lowlevel(m_functionSymbol, args.getBuffer());
    }
    catch (std::exception& e) {
		std::string uw = e.what();
		dbg::trace << "// @FIRST-CHANCE-EXCEPTION: " << uw << dbg::endl;
		throw UserExceptionOccurredException(uw);
    }
    catch (...) {
		throw UserExceptionOccurredException();
    }
}

/**
 * Implements a medium-level call. The arguments in
 * the actual arguments list are translated using the stored
 * <classref>TypeOfArgument</classref>s in the prototype and pushed
 * on an internal <classref>ArgumentsBuffer</classref>, which is
 * then passed to the corresponding low-level call. The return value
 * is also translated, provided that a return type was specified.
 * <p>If the number of arguments mismatches,
 * <classref>InvalidArgumentsException</classref> is thrown.
 */
scripting_element CFunction::call(const ActualArgumentList& args) const
{
	return call(args.size(), &*args.begin());
}

/**
 * Implements a medium-level call. The arguments in
 * the actual arguments list are translated using the stored
 * <classref>TypeOfArgument</classref>s in the prototype and pushed
 * on an internal <classref>ArgumentsBuffer</classref>, which is
 * then passed to the corresponding low-level call. The return value
 * is also translated, provided that a return type was specified.
 * <p>If the number of arguments mismatches,
 * <classref>InvalidArgumentsException</classref> is thrown.
 * (this is an optimization version of call(ActualArgumentList) )
 */
scripting_element CFunction::call(size_t nargs, 
								  const ActualArgumentArray args) const
{
    // Check the number of arguments
    if (nargs != m_formalArguments.size())
		throw InvalidArgumentsException();

    // Push arguments in a buffer
    ArgumentsBuffer argsbuf;

    for (unsigned int index = 0; index < nargs; ++index) {
		// Translate the argument
		m_formalArguments[index]->put(argsbuf, args[index]);
    }

    // Invoke
    basic_block return_value;
    return_value = call(argsbuf);

    // Translate return value
    if (m_returnType)
		return m_returnType->get(return_value);
    else
		return NONE;
}





/**
 */
UserExceptionOccurredException::UserExceptionOccurredException()
    : user_what("??")
{ }

/**
 */
UserExceptionOccurredException::UserExceptionOccurredException(std::string w)
    : user_what(w)
{ }

UserExceptionOccurredException::~UserExceptionOccurredException() throw () { }

} // end of namespace Robin

