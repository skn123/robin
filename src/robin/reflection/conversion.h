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
#include "types.h"
#include "../debug/trace.h"

#ifdef INFINITE
#undef INFINITE
#endif

namespace Robin {

class GarbageCollection;
class RobinType;

/**
 * @class Conversion
 * @nosubgrouping
 *
 * Base class for conversions. Stores the source and destination
 * types as well as a field for the weight. By deriving and overriding
 * the pure virtual <methodref>convert()</methodref> method, many
 * types of conversions may be implemented.
 *
 * Notice that a conversion can be a const conversion when the
 * target type of the conversion is constant. In that case the
 * conversion is one directional, values wont be copied back when
 * the called function returns.
 *
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
	 * @name Destructors
	 */

	//@{
	virtual ~Conversion() = 0;

	//@}




	/**
	 * @name Access
	 */

	//@{
	void setSourceType(Handle<RobinType> type);
	void setTargetType(Handle<RobinType> type);
	void setWeight(const Weight& w);

	Handle<RobinType> sourceType() const;
	Handle<RobinType> targetType() const;
	const Weight& weight() const;

	virtual bool isZeroWorkConversion() const;

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
		/**
		 * This constructor generates an undefined
		 * weight
		 */
		Weight();

		/**
		 * This is the real constructor which defines a
		 * weight
		 */
		Weight(int eps, int prom, int up, int user);  /* explicit setting */

		bool operator <(const Weight& other) const;
		bool operator <=(const Weight& other) const;
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
		friend Robin::dbg::TraceSink &operator<<(Robin::dbg::TraceSink &out, const Conversion::Weight&type);
		int m_n_epsilon;
		int m_n_promotion;
		int m_n_upcast;
		int m_n_userDefined;
#ifndef NDEBUG
		/**
		 * Whatever this weight object is unknown or defined
		 */
		bool m_set;
#endif
	};

private:
	Handle<RobinType> m_source;
	Handle<RobinType> m_target;
	Weight                 m_weight;
};


typedef std::vector<Conversion::Weight>       WeightList;
Robin::dbg::TraceSink &operator<<(Robin::dbg::TraceSink &out, const Conversion::Weight&type);

/**
 * @class ConversionRoute
 * @nosubgrouping
 *
 * It describes how to convert from one type to another and the
 * weight of doing that.
 * It contains a concatanation of several conversions applied one
 * after another.
 *
 * Some zeroWork conversions might have been eliminated
 * for the sake of efficiency but their weight is still included
 * in the weight of the ConversionRoute.
 */
class ConversionRoute : public std::vector<Handle<Conversion> >
{
public:
	/**
	 * @name Decision Support
	 */

	//@{
	Conversion::Weight totalWeight() const;
	//@}


	/**
	 * If some conversions were removed because they do no work
	 * we still need to add their weight to the conversion
	 */
	void addExtraWeight(const Conversion::Weight &amount);


	/**
	 * Returns true if all the conversions composing the route
	 * are one directional.
	 */
	bool hasOnlyConstantConversions() const;

	/**
	 * Returns true if all the internal conversions are zeroWorkConversions.
	 * This means it will be true if they just return a new reference to
	 * the same value without doing actual work.
	 */
	bool isZeroWorkConversionRoute() const;

	/**
	 * @name Activity
	 */
	//@{
	scripting_element apply(scripting_element value,
							GarbageCollection& temporaryHeap) const;
	//@}

	/**
	 * @name Constructors
	 */
	//@{
		ConversionRoute()
			:m_extra(Conversion::Weight::ZERO)
		{

		}
	//@}
private:
	Conversion::Weight m_extra;
};

/**
 * This class represents a list of several independent conversion routes.
 * Ussually used to represent conversion routes for each parameter of a function.
 */
class ConversionRoutes : public std::vector<Handle<ConversionRoute> >
{
public:
	bool areAllEmptyConversions();
};

} // end of namespace Robin

#endif
