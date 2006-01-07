// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @par SOURCE
 * extreme/test_reflection_conversions.cc
 *
 * @par TITLE
 * Conversions
 *
 * Tests the conversions mechanism.
 */

#include <assert.h>

#include <robin/reflection/typeofargument.h>
#include <robin/reflection/cfunction.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/overloadedset.h>
#include <robin/frontends/simple/elements.h>
#include <robin/reflection/conversion.h>
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/memorymanager.h>

#include "fwtesting.h"
#include "reflectiontest.h"

using namespace Extreme;

/**
 * @par TEST
 * TestSimpleConversion
 *
 * Makes sure that the conversions mechanism knows how
 * to convert from int to long and back.
 */
class TestSimpleConversion : public ReflectionTest
{
private:
	Robin::scripting_element source_element;
	Robin::scripting_element converted_element;
	Robin::GarbageCollection gc;

	long converted_value;
	long alt_value;

public:
	TestSimpleConversion() : ReflectionTest("Simple Conversions") { }

	void prepare() {
		source_element = randomElementInt();
	}

	void go() {
		// Get a conversion route
		Handle<Robin::ConversionRoute> route =
			Robin::ConversionTableSingleton::getInstance()
			->bestSingleRoute(*Robin::ArgumentInt, *Robin::ArgumentLong);
		assert(route);
		// Apply conversion
		converted_element = route->apply(source_element, gc);
		// Grab value from within
		converted_value = ((Simple::Long*)converted_element)->value;
	}


	void alternate() {
		// Just convert an int to long normally
		alt_value = ((Simple::Integer*)source_element)->value;
	}

	bool verify() {
		return (converted_value == alt_value);
	}
	
} __t11;
