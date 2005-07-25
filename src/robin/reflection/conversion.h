// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/conversion.h
 *
 * @par TITLE
 * Conversion Base
 *
 * @par PACKAGE
 * Robin
 *
 * <p>Defines the basic representation of the implicit conversion
 * mechanism: types are represented as nodes in a graph, in which the edges
 * represent possible conversions. Each edge has a weight, indicating the
 * "price" of the corresponding conversion. Trivial casts such as promotion
 * and up-cast are assigned low weights; user-defined conversions are more
 * expensive.</p>
 * When observing two types between which there should be a conversion made,
 * the problem can be then reduced to that of finding a shortest path in
 * a directional graph. One possible approach is represented in other sources
 * such as <sourceref>reflection/implicit_conversion_mechanism.cc</sourceref>.
 */

#ifndef ROBIN_REFLECTION_CONVERSION_H
#define ROBIN_REFLECTION_CONVERSION_H

// STL includes
#include <vector>

// Pattern includes
#include <pattern/handle.h>

// Package includes
#include "typeofargument.h"
#include "insight.h"


#ifdef INFINITE
#undef INFINITE
#endif

namespace Robin {

class GarbageCollection;

/**
 * @class Conversion
 * @nosubgrouping
 *
 * Base class for conversions. Stores the source and destination
 * types as well as a field for the weight. By deriving and overriding
 * the pure virtual <methodref>convert()</methodref> method, many
 * types of conversions may be implemented.
 */
class Conversion
{
public:
	class Weight;

	/**
	 * @name Constructors
	 */

	//@{
	Conversion();

	//@}
	/**
	 * @name Access
	 */

	//@{
	void setSourceType(Handle<TypeOfArgument> type);
	void setTargetType(Handle<TypeOfArgument> type);
	void setWeight(const Weight& w);

	Handle<TypeOfArgument> sourceType() const;
	Handle<TypeOfArgument> targetType() const;
	const Weight& weight() const;

	virtual Weight weight(Insight insight) const;

	//@}

	/**
	 * @name Activity
	 */

	//@{

	/**
	 * Defines the core behavior of the conversion
	 * entity. The argument 'o' is assumed to be of the source
	 * type, and the return value should be of the target type.
	 */
	virtual scripting_element apply(scripting_element value) const = 0;
	//@}

	/**
	 * @class Weight
	 * @nosubgrouping
	 *
	 * The weight of a single conversion is denoted
	 * by the number of conversions required for each type:
	 * <ul>
	 *   <li>Trivial conversion (epsilon)</li>
	 *   <li>Promotion</li>
	 *   <li>Up-cast</li>
	 *   <li>User-defined conversion</li>
	 * </ul>
	 * Weights are compared lexicographically, thus, the number 
	 * of user-defined conversions is compared first, then the
	 * number of up-cast conversions and so forth.
	 */
	class Weight
	{
	public:
		Weight();
		Weight(int eps, int prom, int up, int user);  /* explicit setting */

		bool operator <(const Weight& other) const;

		Weight operator +(const Weight& other) const;
		Weight& operator +=(const Weight& other);

		bool isPossible() const;

		inline int getEpsilon()     const { return m_n_epsilon;     }
		inline int getPromotion()   const { return m_n_promotion;   }
		inline int getUpcast()      const { return m_n_upcast;      }
		inline int getUserDefined() const { return m_n_userDefined; }

		static const Weight INFINITE;
		static const Weight ZERO;
		static const Weight UNKNOWN;

		void dbgout() const;

	private:
		int m_n_epsilon;
		int m_n_promotion;
		int m_n_upcast;
		int m_n_userDefined;
	};

private:
	Handle<TypeOfArgument> m_source;
	Handle<TypeOfArgument> m_target;
	Weight                 m_weight;
};

/**
 * @class ConversionRoute
 * @nosubgrouping
 *
 * A concatanation of several conversions applied one
 * after another.
 */
class ConversionRoute : public std::vector<Handle<Conversion> >
{
public:
	/**
	 * @name Decision Support
	 */

	//@{
	Conversion::Weight totalWeight() const;
	Conversion::Weight totalWeight(Insight insight) const;
	//@}

	/**
	 * @name Activity
	 */

	//@{
	scripting_element apply(scripting_element value,
							GarbageCollection& temporaryHeap) const;
	//@}
};


} // end of namespace Robin

#endif
