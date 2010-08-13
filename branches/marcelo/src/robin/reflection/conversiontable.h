// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/conversiontable.h
 *
 * @par TITLE
 * Conversion Table and the Conversion Mechanism
 *
 * @par PACKAGE
 * Robin
 *
 * The conversions form a graph, in which the nodes are the
 * <classref>TypeOfArgument</classref>s and the edges are 
 * <classref>Conversion</classref>s. Here, this general data structure is
 * put together.
 *
 * @par PUBLIC-CLASSES
 * 
 * <ul>
 *   <li>ConversionTable</li>
 *   <li>ConversionTableSingleton</li>
 *   <li>NoApplicableConversionException</li>
 * </ul>
 */

#ifndef ROBIN_REFLECTION_CONVERSIONMECH_H
#define ROBIN_REFLECTION_CONVERSIONMECH_H

/**
 * \@INCLUDES
 */

// STL includes
#include <map>
#include <vector>
#include <list>
#include <exception>

// Pattern includes
#include <pattern/singleton.h>
#include <pattern/handle.h>

// Package includes
#include "conversion.h"
#include "typeofargument.h"
#include "insight.h"


namespace Robin {

/**
 * @class ConversionTable
 * @nosubgrouping
 *
 * Connects nodes in the conversion graph by associating
 * an adjacency list with <classref>TypeOfArgument</classref> objects.
 * The address of the objects is used as a uniquely identifying key
 * in a map.<br />
 * This class is a singleton. Use the
 * <classref>ConversionTableSingleton<classref> identifier to acquire
 * the global instance.
 */
class ConversionTable
{
public:
	struct Adjacency 
	{
		Handle<TypeOfArgument> targetNode;
		Handle<Conversion>     edge;
	};
	typedef std::list<Adjacency> AdjacencyList;

	ConversionTable();
	~ConversionTable();

	/**
	 * @name Declaration API
	 */

	//@{
	void registerConversion(Handle<Conversion> edge);
	void registerEdgeConversion(Handle<Conversion> exit);
	//@}

	/**
	 * @name Access
	 */

	//@{
	const AdjacencyList& getAdjacentTo(const TypeOfArgument& node) const;
	Handle<Conversion> getEdgeConversion(const TypeOfArgument& node) const;
	//@}

	/**
	 * @name Algorithmetics
	 */

	//@{
	Handle<ConversionRoute> bestSingleRoute(const TypeOfArgument& from,
											const TypeOfArgument& to) const;

	Handle<ConversionRoute> bestSingleRoute(const TypeOfArgument& from,
											const TypeOfArgument& to,
											Insight insight) const;

	void bestSequenceRoute
		(const std::vector<Handle<TypeOfArgument> >& from_seq,
		 const std::vector<Handle<TypeOfArgument> >& to_seq,
		 std::vector<Handle<ConversionRoute> >& result_seq) const;

	void bestSequenceRoute
		(const std::vector<Handle<TypeOfArgument> >& from_seq,
		 const std::vector<Insight>& insight_seq,
		 const std::vector<Handle<TypeOfArgument> >& to_seq,
		 std::vector<Handle<ConversionRoute> >& result_seq) const;

	// - optimization version
	void bestSequenceRoute(Handle<TypeOfArgument> from_seq[],
						   const std::vector<Handle<TypeOfArgument> >& to_seq,
						   Handle<ConversionRoute> result_seq[]) const;

	void forceRecompute();
	//@}

private:
	void addTrivialConversions(
			AdjacencyList &list, const TypeOfArgument*) const;
	
	
	typedef std::map<const TypeOfArgument *, AdjacencyList> adjmap;
	typedef std::map<const TypeOfArgument *, Handle<Conversion> > exitmap;
	adjmap m_graph;
	exitmap m_edgeconv;

	class Cache;
	Cache *m_cache;
};


/**
 * @class NoApplicableConversionException
 * @nosubgrouping
 *
 * Occurs when no path of conversions connect the source
 * type and the destination type when calling <tt>bestSingleRoute</tt>
 * or <tt>bestSequenceRoute</tt>.
 */
class NoApplicableConversionException : public std::exception
{
public:
	NoApplicableConversionException(const TypeOfArgument *from,
									const TypeOfArgument *to);

	const char *what() const throw();

	const TypeOfArgument *from;
	const TypeOfArgument *to;
};

/**
 * @class ConversionTableSingleton
 * @nosubgrouping
 *
 * Provides access to the single instance of a conversion
 * table which exists in the reflection subsystem. This instance
 * serves all conversion-related objects, including 
 * <classref>OverloadedSet</classref>s.
 */
class ConversionTableSingleton : public Pattern::Singleton<ConversionTable>
{
public:
	static ConversionTable *getInstance();
};

} // end of namespace Robin


#endif
