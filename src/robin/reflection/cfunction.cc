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

#include <algorithm>
#include <cassert>

#include "cfunction.h"

#include "backtrace.h"
#include "error_handler.h"
#include "../debug/trace.h"
#include <robin/frontends/framework.h>
#include <stdexcept>
namespace Robin {





/**
 * Low-level constructor, builds function directly with
 * an address of a native C function (rather than resolving the
 * symbol from the Library).
 */
CFunction::CFunction(symbol function, std::string functionName, CFunction::FunctionKind functionKind, std::string className)
    : m_allow_edge(true), m_functionSymbol(function),  m_returnIsOwner(true),
    m_functionName(functionName) , m_funcKind(functionKind), m_className(className)
{

}

CFunction::CFunction(symbol function, std::string functionName, CFunction::FunctionKind functionKind)
    : m_allow_edge(true), m_functionSymbol(function),  m_returnIsOwner(true),
    m_functionName(functionName) , m_funcKind(functionKind)
{

}


CFunction::~CFunction()
{
}


/**
 * Sets the expected type of the value returned from
 * the function, as a <classref>RobinType</classref> object.
 */
void CFunction::specifyReturnType(Handle<RobinType> type)
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
				  Handle<RobinType> type)
{
    m_formalArguments.push_back(type);
    m_formalArgumentNames.push_back(name);
    // assume that there are no arguments
    // with same name.
    // XXX: perhaps add a check, wouldn't be
    // difficult, and could be a sanity
    m_formalArgumentNamePositionMap[name] = m_formalArguments.size()-1;
}

/**
 * Declares an anonymous argument (equivalent to
 * <code>addFormalArgument("", type)</code>.
 */
void CFunction::addFormalArgument(Handle<RobinType> type)
{
    addFormalArgument("", type);
}

/**
 * Provides useful information which is important for correct memory
 * management when the C function is called.
 * @param is_return_owner 'true' if the return value is owner of its own
 *   memory, or is the call 'owner' the real owner of the memory
 * @see CFunction::call
 */
void CFunction::supplyMemoryManagementHint(bool is_return_owner)
{
	m_returnIsOwner = is_return_owner;
}





/**
 * Return a list of this function's formal arguments.
 */
const RobinTypes& CFunction::signature() const
{
    return m_formalArguments;
}

/**
 * Returns the type object identified with the function's return type.
 */
Handle<RobinType> CFunction::returnType() const
{
	return m_returnType;
}

/**
 * Returns the argument names
 */
const std::vector<std::string> &CFunction::argNames() const
{
        return m_formalArgumentNames;
}



/**
 * It checks that the number of arguments and their names (if passed by name)
 * are according to this signature.
 *
 * @returns true if the arguments are valid
 *
 */
bool CFunction::checkProperArgumentsPassed(CallRequest &req) const
{


	const std::vector<std::string> &namesOfPassedByName =  req.m_argsPassedByName;

	// The amount of parameters needed by this function
	size_t requiredAmount = this->m_formalArguments.size();

	//Whatever each specific argument was passed
	std::vector<bool> argumentWasPassed(requiredAmount,false);

    if(req.m_nargs != requiredAmount) {
			dbg::trace << "Incorrect number of arguments for this signature requested=" << req.m_nargs << " signature_required=" << requiredAmount << dbg::endl;
            return false;
    }

    // first, go over the nonkw-arguments
    for(size_t index = 0; index<req.m_nargsPassedByPosition; index++)
    {
			assert_true(index == m_formalArgumentNamePositionMap.find(m_formalArgumentNames[index])->second);
            argumentWasPassed[index] = true;
    }


    // then go over the kwargs, verify that they exist, haven't appeared before
    // and don't shadow a positional argument
    for(std::vector<std::string>::const_iterator
											kwiter = namesOfPassedByName.begin();
                                           kwiter != namesOfPassedByName.end();
                                           ++kwiter)
    {
            const std::string &argument_name = *kwiter;

            ArgumentPositionMap::const_iterator argumentPosition = m_formalArgumentNamePositionMap.find(argument_name);


            if(argumentPosition == m_formalArgumentNamePositionMap.end()) {
					dbg::trace << "Unexistant argument " + argument_name + " for this signature" << dbg::endl;
					return false;
            }



            //The keyword args are after the regular args
			int positionInArray = argumentPosition->second;

			dbg::trace << "Parameter " << argument_name << " positioned in " << positionInArray << dbg::endl;

			if(argumentWasPassed[positionInArray]) {
				dbg::trace << "Passed twice the parameter  " + argument_name + " for this signature" << dbg::endl;
            	return false;
            }
            argumentWasPassed[positionInArray] = true;
    }
    return true;
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
	static LowLevel defaultLowLevel;
	
	const LowLevel   *lowlevel = NULL;
	ErrorHandler *errorhandler = NULL;
	try {
		lowlevel     = &(FrontendsFramework::activeFrontend()->getLowLevel());
		errorhandler = &(FrontendsFramework::activeFrontend()->getErrorHandler());
	}
	catch (EnvironmentVacuumException &e)
	{
		lowlevel = &defaultLowLevel;
	}
	
    try {
		return lowlevel->call_lowlevel(m_functionSymbol, args.getBuffer(), args.size());
    }
    catch (std::exception& e) {
		std::string uw = e.what();
		dbg::trace << "// @FIRST-CHANCE-EXCEPTION: " << uw << dbg::endl;
		if (errorhandler) {
			if (!errorhandler->getError()) {
				errorhandler->setError(e, Backtrace::generateFromHere());
			}
		}
		throw UserExceptionOccurredException(uw);
    }
    catch (...) {
		throw UserExceptionOccurredException();
    }
}

/**
 * Implements a medium-level call. The arguments in
 * the actual arguments list are translated using the stored
 * <classref>RobinType</classref>s in the prototype and pushed
 * on an internal <classref>ArgumentsBuffer</classref>, which is
 * then passed to the corresponding low-level call. The return value
 * is also translated, provided that a return type was specified.
 * <p>If the number of arguments mismatches,
 * <classref>InvalidArgumentsException</classref> is thrown.
 */
scripting_element CFunction::call(const ActualArgumentList& args,
                                  scripting_element owner) const
{
	return call(args.size(), (args.size()==0)?0:&*args.begin(), owner);
}

/**
 * Implements a medium-level call. The arguments in
 * the actual arguments list are translated using the stored
 * <classref>RobinType</classref>s in the prototype and pushed
 * on an internal <classref>ArgumentsBuffer</classref>, which is
 * then passed to the corresponding low-level call. The return value
 * is also translated, provided that a return type was specified.
 * <p>If the number of arguments mismatches,
 * <classref>InvalidArgumentsException</classref> is thrown.
 * (this is an optimization version of call(ActualArgumentList) )
 */
scripting_element CFunction::call(size_t nargs,
								  const ActualArgumentArray args,
								  scripting_element owner) const
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
		return owned(m_returnType->get(return_value), owner);
    else
		return NONE;
}


scripting_element CFunction::owned(scripting_element value, scripting_element owner) const
{
	if (!m_returnIsOwner && owner != 0)
		FrontendsFramework::activeFrontend()->own(value, owner);
	return value;
}



std::ostream &operator<<(std::ostream &o,const CFunction &fun)
{

	size_t parameter_index = 0;
	bool removeFirstParam;
	bool removeReturnType;
	switch(fun.m_funcKind)
	{
		case CFunction::GlobalFunction:
			removeFirstParam = false;
			removeReturnType = false;
			break;
		case CFunction::Method:
			removeFirstParam = true;
			removeReturnType = false;
			break;
		case CFunction::StaticMethod:
			removeFirstParam = false;
			removeReturnType = false;
			break;
		case CFunction::Constructor:
			removeFirstParam = false;
			removeReturnType = true;
			break;
		case CFunction::Destructor:
			removeFirstParam = false;
			removeReturnType = true;
			break;
		default:
			throw std::logic_error("Found an unknown type of function\n");

	}

	if(!removeReturnType)
	{
		if(fun.m_returnType) {
			o << *fun.m_returnType << " ";
		} else {
			o << "void ";
		}
	}

	if(fun.m_className.length() > 0) {
		o << fun.m_className << "::";
	}

	if(removeFirstParam) {
		parameter_index = 1;
	}

	o  << fun.m_functionName << "(";
	assert_true(fun.m_formalArguments.size()== fun.m_formalArguments.size());
	bool shown_first_argument = false;
	for (; parameter_index< fun.m_formalArguments.size();parameter_index++) {
		if(shown_first_argument){
			o << ", ";
		} else {
			shown_first_argument = true;
		}
		o << *fun.m_formalArguments[parameter_index] << " " << fun.m_formalArgumentNames[parameter_index];
	}
	o << ")";
	return o;
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

