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

#include "simplefrontend.h"
#include "simpleadapters.h"
#include "simpleconversions.h"

#include <robin/reflection/typeofargument.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/class.h>
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/error_handler.h>
#include "instanceelement.h"


namespace Robin {


class NullErrorHandler : public ErrorHandler
{
public:
	virtual ~NullErrorHandler() { }
	virtual void setError(scripting_element error) { }
	virtual void setError(const std::exception &exc,
						  const Backtrace &trace) { }
	virtual scripting_element getError() { return 0; }
};



/**
 * Default Constructor.
 */
SimpleFrontend::SimpleFrontend()
{
	m_lowLevel = new LowLevel();
	m_errorHandler = new NullErrorHandler();
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
void SimpleFrontend::initialize() const
{
	// Create conversion class instances
	Handle<Conversion> hint2long
		(new SimplePrimitiveConversion<Simple::Integer,long>);
	Handle<Conversion> hint2ulong
		(new SimplePrimitiveConversion<Simple::Integer,unsigned long>);
	Handle<Conversion> hlong2int
		(new SimplePrimitiveConversion<Simple::Long,int>);
	Handle<Conversion> hint2float
		(new SimplePrimitiveConversion<Simple::Integer,float>);
	Handle<Conversion> hint2double
		(new SimpleFakeDoubleConversion<Simple::Integer>);
	Handle<Conversion> hfloat2double
		(new SimpleFakeDoubleConversion<Simple::Float>);
	Handle<Conversion> hstring2pascal
		(new SimpleStringToPascalStringConversion);

	hint2long->setSourceType(ArgumentInt); 
	hint2long->setTargetType(ArgumentLong);
	hint2ulong->setSourceType(ArgumentInt); 
	hint2ulong->setTargetType(ArgumentULong);
	hlong2int->setSourceType(ArgumentLong);
	hlong2int->setTargetType(ArgumentInt);
	hint2float->setSourceType(ArgumentInt);
	hint2float->setTargetType(ArgumentFloat);
	hint2double->setSourceType(ArgumentInt);
	hint2double->setTargetType(ArgumentDouble);
	hfloat2double->setSourceType(ArgumentFloat);
	hfloat2double->setTargetType(ArgumentDouble);
	hstring2pascal->setSourceType(ArgumentCString);
	hstring2pascal->setTargetType(ArgumentPascalString);

	// Register the conversions
	ConversionTableSingleton::getInstance()->registerConversion(hint2long);
	ConversionTableSingleton::getInstance()->registerConversion(hint2ulong);
	ConversionTableSingleton::getInstance()->registerConversion(hlong2int);
	ConversionTableSingleton::getInstance()->registerConversion(hint2float);
	ConversionTableSingleton::getInstance()->registerConversion(hfloat2double);
	ConversionTableSingleton::getInstance()->registerConversion(hstring2pascal);
}



/**
 * Returns the <classref>TypeOfArgument</classref>
 * representing this element's type. The 'element' is assumed to
 * be a <code>Simple::Element *</code>.
 */
Handle<TypeOfArgument> SimpleFrontend::detectType(scripting_element element)
  const
{
	Simple::Element *se = reinterpret_cast<Simple::Element *>(element);
	SimpleInstanceObjectElement *sioe;
	SimpleAddressElement *sae;
	SimpleEnumeratedConstantElement *sece;

	if (dynamic_cast<Simple::Integer*>(se))      return ArgumentInt;
	else if (dynamic_cast<Simple::Long*>(se))    return ArgumentLong;
	else if (dynamic_cast<Simple::String*>(se))  return ArgumentCString;
	else if (dynamic_cast<Simple::Char*>(se))    return ArgumentChar;
	else if (dynamic_cast<Simple::Float*>(se))   return ArgumentFloat;
	else if (dynamic_cast<PascalStringElement*>(se))  
		                                         return ArgumentPascalString;
	else if (sioe = dynamic_cast<SimpleInstanceObjectElement*>(se))
		return sioe->value->getClass()->getRefArg();
	else if (sae = dynamic_cast<SimpleAddressElement*>(se))
		return sae->value->getPointerType();
	else if (sece = dynamic_cast<SimpleEnumeratedConstantElement*>(se))
		return sece->value.getType()->getArg();
	else
		throw UnsupportedInterfaceException();
}


/**
 * Returns an insight for a type.
 *
 * @note the simple frontend does not supply any insights, so the returned
 * value is always 0.
 */
Insight SimpleFrontend::detectInsight(scripting_element element) const
{
	Insight insight;
	insight.i_long = 0;
	return insight;
}


/**
 * Factorize and give an <classref>Adapter</classref>
 * suitable for converting elements of the specified type.<br />
 * Practically, this method returns one of:
 * <ul>
 *   <li><classref>SimpleIntegerAdapter</classref></li>
 *   <li><classref>SimpleLongAdapter</classref></li>
 *   <li><classref>SimpleInstanceObjectAdapter</classref></li>
 * </ul>
 */
Handle<Adapter> SimpleFrontend::giveAdapterFor(const TypeOfArgument& type)
  const
{
	Type basetype = type.basetype();

	if (type.isPointer()) {
		Handle<TypeOfArgument> pointedType = 
			TypeOfArgument::handleMap.acquire(type.pointed());
		if (pointedType)
			return Handle<Adapter>(new SimpleAddressAdapter(pointedType));
		else
			throw UnsupportedInterfaceException();
	}
	else if (basetype.category == TYPE_CATEGORY_INTRINSIC)
	{
		if (basetype.spec == TYPE_INTRINSIC_INT)
			return Handle<Adapter>(new SimpleAdapter<int, Simple::Integer>);
		else if (basetype.spec == TYPE_INTRINSIC_LONG)
			return Handle<Adapter>(new SimpleAdapter<long, Simple::Long>);
		else if (basetype.spec == TYPE_INTRINSIC_ULONG)
			return Handle<Adapter>(new SimpleAdapter<ulong, Simple::Long>);
		else if (basetype.spec == TYPE_INTRINSIC_FLOAT)
			return Handle<Adapter>(new SimpleAdapter<float, Simple::Float>);
		else if (basetype.spec == TYPE_INTRINSIC_DOUBLE)
			return Handle<Adapter>(new SimpleLargeAdapter<double,
								                          FakeDoubleElement>);
		else if (basetype.spec == TYPE_INTRINSIC_CHAR)
			return Handle<Adapter>(new SimpleAdapter<char, Simple::Char>);
		else if (basetype.spec == TYPE_INTRINSIC_BOOL)
			return Handle<Adapter>(new SimpleAdapterWithCast<bool, 
								                             Simple::Integer>);
		else
			throw UnsupportedInterfaceException();
	}
	else if (basetype.category == TYPE_CATEGORY_EXTENDED)
	{
		if (basetype.spec == TYPE_EXTENDED_CSTRING)
			return Handle<Adapter>(new SimpleCStringAdapter);
		if (basetype.spec == TYPE_EXTENDED_PASCALSTRING)
			return Handle<Adapter>(new SimplePascalStringAdapter);
		else
			throw UnsupportedInterfaceException();
	}
	else if (basetype.category == TYPE_CATEGORY_USERDEFINED)
	{
		if (basetype.spec == TYPE_USERDEFINED_OBJECT)
		 return Handle<Adapter>(new SimpleInstanceAdapter(basetype.objclass));
		else if (basetype.spec == TYPE_USERDEFINED_ENUM)
		 return Handle<Adapter>(new SimpleEnumeratedAdapter(basetype.objenum));
		else
			throw UnsupportedInterfaceException();
	}
	else
		throw UnsupportedInterfaceException();
}



/**
 * Does nothing. There is no actual interpreter at the
 * other side of the SimpleFrontend.
 */
void SimpleFrontend::exposeLibrary(const Library& newcomer)
{
}

/**
 * Does nothing. The SimpleFrontend implements no garbage collection -
 * allocated objects will thus never be freed! Be warned.
 */
scripting_element SimpleFrontend::duplicateReference(scripting_element element)
{
	return element;
}

/**
 * Does nothing. The SimpleFrontend implements no garbage collection -
 * allocated objects will thus never be freed! Be warned.
 */
void SimpleFrontend::release(scripting_element element)
{
}

/**
 * Does nothing. The SimpleFrontend implements no garbage collection -
 * allocated objects will thus never be freed, hence no memory ownership is
 * required.
 */
void SimpleFrontend::own(scripting_element master, scripting_element slave)
{
}

/**
 * Does nothing. The SimpleFrontend implements no garbage collection -
 * allocated objects will thus never be freed, hence no memory ownership is
 * required.
 */
void SimpleFrontend::bond(scripting_element master, scripting_element slave)
{
}

/**
 * Returns the low level interface for the low level function calls.
 */
const LowLevel& SimpleFrontend::getLowLevel() const
{
	return *m_lowLevel;
}

/**
 * Returns the interceptor interface for the low level callbacks.
 */
const Interceptor& SimpleFrontend::getInterceptor() const
{
	assert(false);
}

/**
 * Returns the error handler for this frontend.
 */
ErrorHandler& SimpleFrontend::getErrorHandler()
{
	return *m_errorHandler;
}

/**
 * Destructor
 */
SimpleFrontend::~SimpleFrontend()
{
	delete m_lowLevel;
}

} // end of namespace Robin
