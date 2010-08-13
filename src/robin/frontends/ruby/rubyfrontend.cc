// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/simple/simplefrontend.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "rubyfrontend.h"
#include "rubyadapters.h"

#include <cassert>
#include <iostream>

// Ruby includes
#include <ruby.h>
#include <node.h>

// Package includes
#include <robin/reflection/robintype.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/class.h>
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/error_handler.h>

#include <robin/reflection/library.h>
#include <robin/reflection/namespace.h>

#include "rubyobjects.h"


using namespace Robin::Ruby;


namespace Robin {


class RubyErrorHandler : public ErrorHandler
{
public:
	virtual ~RubyErrorHandler() { }
	virtual void setError(scripting_element error) { }
	virtual void setError(const std::exception &exc,
						  const Backtrace &trace) { }
	virtual scripting_element getError() { return 0; }
};



/**
 * Default Constructor.
 */
RubyFrontend::RubyFrontend()
{
	m_lowLevel = new LowLevel();
	m_errorHandler = new RubyErrorHandler();
}

/**
 * Applies the standard global conversions which are
 * currently available:
 * <ul>
 *  <li>int  -> long  </li>
 *  <li>long -> int   </li>
 *  <li>int  -> float </li>
 * </ul>
 */
void RubyFrontend::initialize() const
{
}



/**
 * Returns the RobinType representing this element's type. The
 * 'element' is assumed to be a Ruby VALUE.
 */
Handle<RobinType> RubyFrontend::detectType_mostSpecific(scripting_element element)
  const
{
	VALUE value = reinterpret_cast<VALUE>(element);

	if (TYPE(value) == T_FIXNUM)
		return ArgumentLong;
	else
		throw UnsupportedInterfaceException();
}




/**
 * Factorize and give an <classref>Adapter</classref>
 * suitable for converting elements of the specified type.<br />
 * Practically, this method returns one of:
 * <ul>
 *   <li>RubyIntegerAdapter</li>
 *   <li>RubyLongAdapter</li>
 *   <li>RubyInstanceObjectAdapter</li>
 * </ul>
 */
Handle<Adapter> RubyFrontend::giveAdapterFor(const RobinType& type)
  const
{
	Type basetype = type.basetype();

	if (type.isPointer()) {
		throw UnsupportedInterfaceException();
	}
	else if (basetype.category == TYPE_CATEGORY_INTRINSIC)
	{
		if (basetype.spec == TYPE_INTRINSIC_LONG)
			return Handle<Adapter>(new RubyLongAdapter);
		else if (basetype.spec == TYPE_INTRINSIC_INT)
			return Handle<Adapter>(new RubyLongAdapter);
		else
			throw UnsupportedInterfaceException();
	}
	else if (basetype.category == TYPE_CATEGORY_EXTENDED)
	{
		throw UnsupportedInterfaceException();
	}
	else if (basetype.category == TYPE_CATEGORY_USERDEFINED)
	{
		throw UnsupportedInterfaceException();
	}
	else
		throw UnsupportedInterfaceException();
}


/**
 * Creates a module to represent the library in Ruby, containing the classes
 * and functions within.
 */
void RubyFrontend::exposeLibrary(const Library& newcomer)
{
	VALUE module = rb_define_module(newcomer.name().c_str());

	Handle<Namespace> global = newcomer.globalNamespace();
	typedef Namespace::NameIterator NameIterator;

	for (Handle<NameIterator> func_it = global->enumerateFunctions();
		 !func_it->done(); func_it->next()) {
		std::string name = func_it->value();
		std::string aname = "@" + name;

		Handle<Callable> func = global->lookupFunction(name);
		dbg::trace << "Introducing function " << name<< dbg::endl;

		VALUE rb_func = FunctionObject::allocate(&*func);
		rb_attr(rb_singleton_class(module), rb_intern(name.c_str()),
				1, 1, NOEX_PUBLIC);
		rb_iv_set(module, aname.c_str(), rb_func);
	}
}

/**
 * Does nothing. The RubyFrontend implements no garbage collection -
 * allocated objects will thus never be freed! Be warned.
 */
scripting_element RubyFrontend::duplicateReference(scripting_element element)
{
	return element;
}

/**
 * Does nothing. The RubyFrontend implements no garbage collection -
 * allocated objects will thus never be freed! Be warned.
 */
void RubyFrontend::release(scripting_element element)
{
}

/**
 * Does nothing. The RubyFrontend implements no garbage collection -
 * allocated objects will thus never be freed, hence no memory ownership is
 * required.
 */
void RubyFrontend::own(scripting_element master, scripting_element slave)
{
}

/**
 * Does nothing. The RubyFrontend implements no garbage collection -
 * allocated objects will thus never be freed, hence no memory ownership is
 * required.
 */
void RubyFrontend::bond(scripting_element master, scripting_element slave)
{
}

/**
 * Returns the low level interface for the low level function calls.
 */
const LowLevel& RubyFrontend::getLowLevel() const
{
	return *m_lowLevel;
}

/**
 * Returns the interceptor interface for the low level callbacks.
 */
const Interceptor& RubyFrontend::getInterceptor() const
{
	assert_true(false);
}

/**
 * Returns the error handler for this frontend.
 */
ErrorHandler& RubyFrontend::getErrorHandler()
{
	return *m_errorHandler;
}

/**
 * Destructor
 */
RubyFrontend::~RubyFrontend()
{
	delete m_lowLevel;
}

} // end of namespace Robin
