// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/simple/pythonconversions.h
 *
 * @par TITLE
 * Sub-classes of Conversion for the Python Interpreter
 *
 * @par PACKAGE
 * Robin
 *
 * Basic conversions which are supplied by the Python Interpreter
 * for built-in conversions between types.
 *
 * @par PUBLIC-CLASSES
 * <ul><li>TrivialConversion</li><li>StringToPascalStringConversion</li></ul>
 */

#ifndef ROBIN_FRONTENDS_PYTHON_CONVERSION_PYTHON_H
#define ROBIN_FRONTENDS_PYTHON_CONVERSION_PYTHON_H

// System includes
#include <assert.h>

// Package includes
#include <robin/reflection/conversion.h>
#include <robin/reflection/fundamental_conversions.h>


namespace Robin {

namespace Python {

class PascalStringToCStringConversion : public TrivialConversion
{
public:
	/**
	 * @name Constructors
	 */
	//@{
	PascalStringToCStringConversion() { setWeight(Conversion::Weight(0,2,0,0)); }
	//@}
};


class IntToFloatConversion : public Conversion
{
public:
	/**
	 * @name Constructors
	 */
	//@{
	IntToFloatConversion() { setWeight(Conversion::Weight(0,1,0,0)); }
	//@}

	/**
	 * @name Activity
	 */
	//@{
	scripting_element apply(scripting_element value) const;
	//@}

};



} // end of namespace Python

} // end of namespace Robin


#endif
