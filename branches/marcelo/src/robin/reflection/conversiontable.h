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
 * <classref>RobinType</classref>s and the edges are
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



namespace Robin {


class RobinType;
class Conversion;
class ConversionTree;
class ConversionRoute;


/**
 * @class ConversionTable
 * @nosubgrouping
 *
 * Connects nodes in the conversion graph by associating
 * an adjacency list with <classref>RobinType</classref> objects.
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
		Handle<RobinType> targetNode;
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
	const AdjacencyList& getAdjacentTo(const RobinType& node) const;
	Handle<Conversion> getEdgeConversion(const RobinType& node) const;
	//@}

	/**
	 * @name Algorithmetics
	 */

	//@{
	/**
	 * It returns the lightest conversion route between two types.
	 * Each Conversion has a related Conversion::Weight, the
	 * weight of the route is the sum of the conversion weights.
	 * Notice that conversions that do zero work (Conversion::isZeroWorkConversion)
	 * will not be included in the conversion route.
	 * For example conversion from subtypes to their supertypes wont
	 * be included.
	 *
	 *	 @throws NoApplicableConversionException if 'to' cannot be
	 *	 reached
	 *
	 * Basically is implemented using generateConversionTree.
	 */
	Handle<ConversionRoute> bestSingleRoute(const RobinType& from,
											const RobinType& to) const;

	/**
	 * It returns a conversion tree from a specific source.
	 * The tree explains compactly how to convert from a specific
	 * source to all the reachable targets.
	 * The tree can be composed of const conversions or regular conversions.
	 *
	 * There is an optional parameter if we want to stop generating the
	 * tree at the moment we are able to convert to a specific type.
	 *
	 * It is implemented using the dijkstra algorithm of
	 * function bestSingleRoute which calculates the one-to-many
	 * shortest path.
	 *
	 * @param source the source type to generate the tree
	 * @param stopType if not null it indicates a type, which when
	 * 			found will make the algorithm stop. All the types
	 * 			which  can be reached at a lighter weight than 'stopType'
	 * 			will also be part of the tree.
	 * 			There is no warranty that 'stopType' will be reached
	 * 			at all.
	 * @param constConversionTree true if the caller needs a tree
	 * 			of one-direction conversions.
	 */
	Handle<ConversionTree> generateConversionTree(const RobinType &source,
													const RobinType *stopType,
													bool constConversionTree) const;
	void forceRecompute();
	//@}

private:


	
	
	typedef std::map<const RobinType *, AdjacencyList> adjmap;
	typedef std::map<const RobinType *, Handle<Conversion> > exitmap;
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
	NoApplicableConversionException(const RobinType *from,
									const RobinType *to);

	const char *what() const throw();

	const RobinType *from;
	const RobinType *to;
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
