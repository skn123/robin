// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/simple/simpleconversions.h
 *
 * @par TITLE
 * Sub-classes of Conversion for the Simple Interpreter
 *
 * @par PACKAGE
 * Robin
 *
 * Basic conversions which are supplied by the Simple Interpreter
 * for built-in conversions between types.
 *
 * @par PUBLIC-CLASSES
 * <ul><li>SimplePrimitiveConversion</li></ul>
 */

#ifndef ROBIN_FRONTENDS_SIMPLE_CONVERSION_SIMPLE_H_MONDAYS
#define ROBIN_FRONTENDS_SIMPLE_CONVERSION_SIMPLE_H_MONDAYS

// Package includes
#include <robin/debug/assert.h>
#include <robin/reflection/conversion.h>
#include "elements.h"
#include "instanceelement.h"


namespace Robin {

/**
 * @class SimplePrimitiveConversion
 * @nosubgrouping
 *
 * Converts values using static C casting (where it's
 * valid).
 */
template < class SourceElement, class TargetCType >
class SimplePrimitiveConversion : public Conversion
{
public:
	/**
	 * @name Constructors
	 */

	//@{

	/**
	 */
	SimplePrimitiveConversion() { setWeight(Conversion::Weight(1,0,0,0)); }

	//@}

	/**
	 * @name Activity
	 */

	//@{

	/**
	 * Uses the C conversion to create the new value.
	 */
	scripting_element apply(scripting_element value) const {
		// Extract the value from the element and convert it in C
		Simple::Element *element = (Simple::Element *)value;
		SourceElement *selement = dynamic_cast<SourceElement *>(element);
		assert(selement);
		TargetCType target_val = (TargetCType)(selement->value);
		// Build an element from this value
		Simple::Element *telement = Simple::build(target_val);
		return (scripting_element)telement;
	}

	//@}
};

/**
 * @class SimpleFakeDoubleConversion
 * @nosubgrouping
 *
 * Converts the original type to double, then stores it
 * in a fake object designed to temporarily store doubles before it
 * is passed to the function.
 */
template < class SourceElement >
class SimpleFakeDoubleConversion : public Conversion
{
public:
	/**
	 * @name Constructors
	 */

	//@{

	/**
	 */
	SimpleFakeDoubleConversion() { setWeight(Conversion::Weight(1,0,0,0)); }

	//@}

	/**
	 * @name Activity
	 */

	//@{

	/**
	 * Uses the C conversion to create the new value.
	 */
	scripting_element apply(scripting_element value) const {
		Simple::Element *element = (Simple::Element *)value;
		SourceElement *selement = dynamic_cast<SourceElement *>(element);
		double target_val = (double)(selement->value);
		// Build an element from this value
		Simple::Element *telement = \
			Simple::buildElement<FakeDoubleElement>(target_val);
		return (scripting_element)telement;
	}
	//@}
};

/**
 * A conversion from Simple::String to PascalStringElement.
 */
class SimpleStringToPascalStringConversion : public Conversion
{
public:
	/**
	 * @name Constructors
	 */

	//@{
	SimpleStringToPascalStringConversion() 
		{ setWeight(Conversion::Weight(1,0,0,0)); }
	//@}

	/**
	 * @name Activity
	 */

	//@{

	/**
	 * Constructs a new Pascal string that points to the same data.
	 */
	scripting_element apply(scripting_element value) const
	{
		Simple::Element *element = (Simple::Element*)value;
		Simple::String *str = static_cast<Simple::String*>(element);
		PascalStringElement *pascal_str = new PascalStringElement;
		pascal_str->value.size = str->value.size();
		pascal_str->value.chars = const_cast<char*>(str->value.c_str());
		return (scripting_element)(Simple::Element*)pascal_str;
	}

	//@}
};

} // end of namespace Robin


#endif
