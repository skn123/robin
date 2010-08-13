// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/fundamental_conversions.h
 *
 * @par TITLE
 * Simple Sub-classes of Conversion
 *
 * @par PACKAGE
 * Robin
 *
 * Some popular, simple derivations of <classref>Conversion
 * </classref> that perform simplistic conversion tasks.
 *
 * @par PUBLIC-CLASSES
 * <ul><li>TrivialConversion</li></ul>
 */

#ifndef ROBIN_REFLECTION_CONVERSION_FUNDAMENTAL_H_MONDAYS
#define ROBIN_REFLECTION_CONVERSION_FUNDAMENTAL_H_MONDAYS

// Package includes
#include "conversion.h"


namespace Robin {

/**
 * @class TrivialConversion
 * @nosubgrouping
 *
 * Practically does nothing, so it is used only to
 * connect between types which are essentially the same and differ
 * only by their semantic meanings - for example, pointers and
 * references to the same class.
 */
class TrivialConversion : public Conversion
{
public: 
	/**
	 * @name Constructors
	 */

	//@{
	TrivialConversion();
	//@}

	/**
	 * @name Access
	 */

	//@{
	virtual bool isZeroWorkConversion() const;
	//@}
	/**
	 * @name Activity
	 */

	//@{
	virtual scripting_element apply(scripting_element value) const;
	//@}
};

/**
 * @class ConversionViaConstruction
 * @nosubgrouping
 *
 * Single-argument constructors may implicitly reflect
 * user-defined conversions; the source value is given as an argument
 * to the constructor of the target type. Extremely useful.
 */
class ConversionViaConstruction : public Conversion
{
public:
	/**
	 * @name Constructors
	 */

	//@{
	ConversionViaConstruction();
	//@}

	/**
	 * @name Activity
	 */

	//@{
	virtual scripting_element apply(scripting_element value) const;
	//@}
};

/**
 * @class UpCastConversion
 * @nosubgrouping
 *
 * An instance pointer may be cast to a base class of
 * its own. A C function is provided to perform the cast as a certain
 * transformation may be applied to the pointer.
 */
class UpCastConversion : public Conversion
{
public:
	typedef void * (*transformfunc) (void*);

	/**
	 * @name Constructors
	 */

	//@{
	UpCastConversion(transformfunc transform);
	//@}

	/**
	 * @name Activity
	 */

	//@{
	virtual scripting_element apply(scripting_element value) const;
	//@}

private:
	transformfunc m_transform;
};


} // end of namespace Robin

#endif
