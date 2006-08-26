// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/conversion.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "conversion.h"
#include "memorymanager.h"

#include <iostream>


namespace Robin {

#define INF 0xFFFF

const Conversion::Weight Conversion::Weight::ZERO(0, 0, 0, 0);
const Conversion::Weight Conversion::Weight::INFINITE(INF, INF, INF, INF);
const Conversion::Weight Conversion::Weight::UNKNOWN(9, 9, 9, 9);





/**
 * Default constructor.
 */
Conversion::Conversion() { }



/**
 * Specifies the source type for the conversion, which
 * is the source of the actual data elements being converted.
 */
void Conversion::setSourceType(Handle<TypeOfArgument> type)
{
	m_source = type;
}

/**
 * Specifies the target type for the conversion, which
 * is the type of output data elements resulted from applying the
 * conversion.
 */
void Conversion::setTargetType(Handle<TypeOfArgument> type)
{
	m_target = type;
}

/**
 * Determines the weight, or cost, of this conversion
 * in terms of <classref>Conversion::Weight</class> structures. See
 * the specific class reference for more information.
 */
void Conversion::setWeight(const Conversion::Weight& w)
{
	m_weight = w;
}

/**
 * Returns the source type of this conversion.
 */
Handle<TypeOfArgument> Conversion::sourceType() const
{
	return m_source;
}

/**
 * Returns the target type of this conversion.
 */
Handle<TypeOfArgument> Conversion::targetType() const
{
	return m_target;
}

/**
 * Returns this conversion's weight (price).
 */
const Conversion::Weight& Conversion::weight() const
{
	return m_weight;
}

/**
 * Returns this conversion's weight (price) being given an insight on the
 * value.
 *
 * @param insight a code providing some information about a value to be
 * converted. The value is implementation defined, and its meaning may change
 * from type to type.
 * @note the default implementation just returns weight().
 */
Conversion::Weight Conversion::weight(Insight insight) const
{
	return weight();
}


/**
 */
Conversion::Weight::Weight()
  : m_n_epsilon(0), m_n_promotion(0), m_n_upcast(0), m_n_userDefined(0)
{ }

/**
 */
Conversion::Weight::Weight(int eps, int prom, int up, int user)
  : m_n_epsilon(eps), m_n_promotion(prom), 
    m_n_upcast(up), m_n_userDefined(user)
{ }

/**
 * Compares two conversion weights to discover which
 * conversion is cheaper. The comparison is done lexicographically,
 * with weight components being the elements. So, first user-defined
 * conversion counter is compared. If the counters are equal, then
 * the upcast conversion counters are checked, and so on.
 */
bool Conversion::Weight::operator < (const Weight &other) const
{
#define NE(F) (F != other.F)
#define CMP(F) (F < other.F)
  if (NE(m_n_userDefined))        return CMP(m_n_userDefined);
  else if (NE(m_n_promotion))     return CMP(m_n_promotion);
  else if (NE(m_n_upcast))        return CMP(m_n_upcast);
  else                            return CMP(m_n_epsilon);
#undef NE
#undef CMP
}

/**
 * Returns the cost of two consequent conversions with
 * the weights given as operands.
 */
Conversion::Weight Conversion::Weight::operator + (const Conversion::Weight& 
												   other) const
{
	Conversion::Weight s;
#define SUM(F) s.F = F + other.F
	SUM(m_n_userDefined);
	SUM(m_n_promotion);
	SUM(m_n_upcast);
	SUM(m_n_epsilon);
#undef SUM
	return s;
}

/**
 * Adds one weight to another. The result is stored in
 * the left operand.
 */
Conversion::Weight& Conversion::Weight::operator += (const Conversion::Weight&
													 other)
{
#define SUM(F) F += other.F;
	SUM(m_n_userDefined);
	SUM(m_n_promotion);
	SUM(m_n_upcast);
	SUM(m_n_epsilon);
#undef SUM
	return *this;
}

/**
 * Checks to see if the weight is not too great for the
 * conversion to take place.
 */
bool Conversion::Weight::isPossible() const
{
	return (m_n_userDefined < 2);
}

void Conversion::Weight::dbgout() const
{
	std::cerr << "(" << m_n_userDefined << "u,"
			  << m_n_promotion << "p," << m_n_upcast << "c," << m_n_epsilon
			  << "e)";
}





/**
 * Returns the sum of edge weights along the path of
 * conversions building up this route.
 */
Conversion::Weight ConversionRoute::totalWeight() const
{
	Conversion::Weight sum = Conversion::Weight::ZERO;

	for (const_iterator iter = begin(); iter != end(); ++iter)
		sum += (*iter)->weight();

	return sum;
}

/**
 * Returns the sum of edge weights along the path of
 * conversions building up this route, enriching the first
 * edge with the knowledge of the insight.
 */
Conversion::Weight ConversionRoute::totalWeight(Insight insight) const
{
	Conversion::Weight sum = Conversion::Weight::ZERO;
	const_iterator iter = begin();

	if (iter != end()) {
		sum += (*iter)->weight(insight);

		for (++iter; iter != end(); ++iter)
			sum += (*iter)->weight();
	}

	return sum;
}



/**
 * Activates the conversions in a pipe on the element
 * being passed. The output of each conversion is forwarded as input
 * to the next one, and the output of the last conversion is the
 * return value.
 * @param value the original value
 * @param temporaryHeap a heap onto which temporary values are placed. Note:
 * you must be done using the result of apply() before cleaning up the
 * garbage!
 * @returns a converted value, this value must be treated as a temporary
 * object (that is, it will be deleted by the garbage collection).
 */
scripting_element ConversionRoute::apply(scripting_element value,
										 GarbageCollection& temporaryHeap)
	const
{
	for (const_iterator iter = begin(); iter != end(); ++iter) {
		value = (*iter)->apply(value);
		temporaryHeap.markForDestruction(value);
	}

	return value;
}


} // end of namespace Robin
