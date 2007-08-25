// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/fundamental_conversions.cc
 *
 * @par TITLE
 * Simple Sub-classes of Conversion
 *
 * @par PACKAGE
 * Robin
 */

#include "fundamental_conversions.h"

#include <assert.h>
#include "class.h"
#include "instance.h"
#include "typeofargument.h"
#include "memorymanager.h"
#include "../frontends/framework.h"


namespace Robin {





/**
 * @par DESCRTIPION
 * Sets the weight of such a conversion to zero by
 * default.
 */
TrivialConversion::TrivialConversion()
{
	setWeight(Conversion::Weight::ZERO);
}



/**
 * Plainly returns the value unchanged.
 */
scripting_element TrivialConversion::apply(scripting_element value) const
{
	return MemoryManager::duplicateReference(value);
}






/**
 * Builds a conversion through a constructor of the
 * target type. Set the source and target type using 
 * <classref>Conversion</classref> methods. No more information is
 * necessary, thus making this conversion type very powerful.
 */
ConversionViaConstruction::ConversionViaConstruction() 
{
	setWeight(Conversion::Weight(0,0,0,1) /* user-defined */);
}



/**
 * Runs the approperiate constructor of the target
 * type; it is assumed to be a class. An <classref>ActualArgumentList
 * </classref> is built comprising of a single argument which is
 * the source value.
 */
scripting_element ConversionViaConstruction::apply(scripting_element value)
	const
{
	// Build argument list = [ value ]
	ActualArgumentList single_arg;
    KeywordArgumentMap nokwargs;
	single_arg.push_back(value);

	// Call constructor
	assert(targetType()->basetype().spec == TYPE_USERDEFINED_OBJECT);
	Handle<Instance> converted =
		targetType()->basetype().objclass->createInstance(single_arg, nokwargs);

	// Create a scripting wrapper
	return converted->scriptify(Instance::OWNER);
}





/**
 * Builds the conversion with a supplied transform
 * function which is used later on to perform the conversion. The
 * transform function is applied to the <b>pointers</b> held in the
 * instance objects, not the Instance or scripting_element objects
 * themselves.
 */
UpCastConversion::UpCastConversion(transformfunc transform)
	: m_transform(transform)
{
	setWeight(Conversion::Weight(0, 0, 1, 0));
}



/**
 * Uses the transform function to create an instance
 * object in which the object is cast to the base class.
 */
scripting_element UpCastConversion::apply(scripting_element value) const
{
	// Construct a C function which processes the source type and
	// outputs the target type
	CFunction func((symbol)m_transform);
	func.addFormalArgument(sourceType());
	func.specifyReturnType(targetType());
	// Call this function to produce converted element
	ActualArgumentList args;
	args.push_back(value);
	scripting_element upcasted = func.call(args);
    // Bond the upcasted to value to avoid double free
    FrontendsFramework::activeFrontend()->bond(value, upcasted);
    return upcasted;
}

} // end of namespace Robin
