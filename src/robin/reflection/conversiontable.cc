// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/conversiontable.cc
 *
 * @par PACKAGE
 * Robin
 */

// Pattern includes
#include <pattern/cache.h>
#include <pattern/assoc_heap.h>

// Package includes
#include "conversiontable.h"
#include "conversiontree.h"
#include "class.h"
#include "enumeratedtype.h"
#include "overloadedset.h"
#include "fundamental_conversions.h"
#include "intrinsic_type_arguments.h"
#include "const.h"

#include "../debug/trace.h"


namespace Robin {


/**
 * Remembers latest routes that were calculated during the existance of
 * this table, so that they can be used later.
 */
class ConversionTable::Cache
{
private:
	struct Key {
		const RobinType *pSource;
		const RobinType *pTarget;
	};

	/**
	 * CacheTraits is a template parameter for Pattern::Cache
	 */
	struct CacheTraits {
		typedef Cache::Key Key;
		typedef Handle<ConversionRoute> Value;
		struct KeyCompareFunctor
		{
			bool operator()(const Key& key1, const Key& key2) const
			{
				return (key1.pSource < key2.pSource) ||
					( (key1.pSource == key2.pSource) &&
					  (key1.pTarget < key2.pTarget) ) ||
					( (key1.pSource == key2.pSource) &&
					  (key1.pTarget == key2.pTarget)  );
			}
		};

		struct KeyIdentityFunctor
		{
			bool operator()(const Key& key1,
							const Key& key2) const
			{
				return (key1.pSource == key2.pSource) &&
				       (key1.pTarget == key2.pTarget);
			}
		};

		struct KeyHashFunctor
		{
			size_t operator()(const Key& key) const {
				return (size_t)key.pSource + (size_t)key.pTarget;
			}
		};



		static inline void dispose(const Key& key)
		{

		}

		static inline const Handle<ConversionRoute> &getMissedElement()
		{
			return Cache::MISSED;
		}

	}; // end of CacheTraitrs

public:
	Pattern::Cache<CacheTraits> actual_cache;

	/**
	 * Store an entry in the cache.
	 */
	inline void remember(const RobinType& source,
						 const RobinType& target,
						 Handle<ConversionRoute> chosenRoute)
	{

		const Key key = { &source, &target};
		actual_cache.remember(key, chosenRoute);
	}

	/**
	 * Fetch an entry from the cache.
	 */
	inline Handle<ConversionRoute> recall(const RobinType& source,
										  const RobinType& target) const
	{
		const Key key = { &source, &target };
		return actual_cache.recall(key);
	}


	inline void flush() { actual_cache.flush(); }

	//The path was stored in the cache as an impossible conversion
	static const Handle<ConversionRoute> IMPOSSIBLE;

	//It will be returned when the conversion is not found in the cache
	static const Handle<ConversionRoute> MISSED;
};

const Handle<ConversionRoute> ConversionTable::Cache::IMPOSSIBLE
                                               (new ConversionRoute);
const Handle<ConversionRoute> ConversionTable::Cache::MISSED;


/**
 * Builds an empty conversion table and initializes cache.
 */
ConversionTable::ConversionTable()
{
	m_cache = new Cache;
}

/**
 * Frees the conversion table.
 */
ConversionTable::~ConversionTable()
{
#ifdef HARD_PROFILE
	fprintf(stderr,
			"Conversion cache debugging report:\n"
			"----------------------------------\n"
			" Hits: %i        Misses: %i\n\n", m_cache->hits, m_cache->misses);
#endif
	delete m_cache;
}

/**
 * Adds a connection between two types through a conversion.
 * The source and target types are both taken from the
 * Conversion object itself, as well as the weight which is stored there by
 * nature.
 */
void ConversionTable::registerConversion(Handle<Conversion> edge)
{

	/*
	 * Each conversion should have a source and a target RobinType.
	 * Edges entering hypergeneric types are not allowed because of
	 * danger of generating loops, anyway there is always a path which
	 * goes first to a specific type and then does the conversions.
	 * The only exception to the hypergenric rule is converting to
	 * the const type of the source,
	 */
#	ifndef NDEBUG
		RobinType *src = edge->sourceType().pointer();
		RobinType *tgt = edge->targetType().pointer();
		assert_true(src);
		assert_true(tgt);
		assert_true(!tgt->isHyperGeneric() ||
				(dynamic_cast<ConstType*>(tgt) &&
				 &dynamic_cast<ConstType*>(tgt)->basetype() == src)
			);
#	endif

	dbg::trace << "Add conversion: '"
			   << *(edge->sourceType()) << "' to '" << *(edge->targetType())
			   << "'" << dbg::endl;




	/* Create an adjacency record */
	Adjacency adj;
	adj.targetNode = edge->targetType();
	adj.edge = edge;
	/* If the source type already exists in the map,
     * add 'edge' to the existing adjacency list. Otherwise,
	 * create a new list and add 'edge' to it. */
	AdjacencyList &adjlist = m_graph[&*(edge->sourceType())];
	adjlist.push_front(adj);

	// - notify change in order to flush outdated caches
	forceRecompute();
	OverloadedSet::forceRecompute(); // do you think this breaks encapsulation?
}


/**
 * Adds an exit edge. An 'edge conversion' is a conversion with only one
 * incident node, the other end pointing to "empty space". An edge conversion
 * incident with a type implies that <b>every time</b> a value of this type
 * is returned from a function call, the associated conversion should be
 * applied (with some exceptions allowed).
 */
void ConversionTable::registerEdgeConversion(Handle<Conversion> exit)
{
	m_edgeconv[&*(exit->sourceType())] = exit;
}


/**
 * Returns a list of nodes (types) which are adjacent
 * to a given node (type). This is done by performing a simple lookup.<br />
 * Types are assumed to implicitly delegate nodes in the graph. If
 * no edges are connected to the specified type, it is just treated as
 * a lonely node with a degree of 0.
 */
const ConversionTable::AdjacencyList&
ConversionTable::getAdjacentTo(const RobinType& node) const
{
	static AdjacencyList the_empty_adjacency_list;  /* returned by default */

	adjmap::const_iterator existing = m_graph.find(&node);

	if (existing != m_graph.end()) {
		return existing->second;
	}
	else {
		return the_empty_adjacency_list;
	}
}

/**
 * Returns an edge conversion incident with the requested type.
 * A null handle is returned if no such conversion was previously registered.
 */
Handle<Conversion>
ConversionTable::getEdgeConversion(const RobinType& node) const
{
	exitmap::const_iterator existing = m_edgeconv.find(&node);

	if (existing != m_edgeconv.end()) {
		return existing->second;
	}
	else {
		return Handle<Conversion>();
	}
}


Handle<ConversionRoute>
ConversionTable::bestSingleRoute(const RobinType& from,
								 const RobinType& to) const
{



	dbg::trace << "Trying to convert from <" << from  << "> (" << &from << ")"
								<< " to <" << to	   << "> (" << &to	<< ")"
								<< dbg::endl;
	dbg::IndentationGuard guard(dbg::trace);

	/* - first we might have some luck and the requested route may be in
	 *   the cache */
	Handle<ConversionRoute> hcached = m_cache->recall(from, to);
	if (hcached != Cache::MISSED)
	{
		if (hcached == Cache::IMPOSSIBLE)
			throw NoApplicableConversionException(&from, &to);
		else
			return hcached;
	}

	/*
	 * If the route was not found in the cache, just generate the
	 * conversion tree.
	 */
	Handle<ConversionTree> previousStepMap = generateConversionTree(from,&to,to.isConstant());


	try {
		/* Reconstruct the path from the source to the destination */
		Handle<ConversionRoute> hroute( previousStepMap->generateRouteTo(to.get_handler()));

#		ifndef NDEBUG
			dbg::trace("@TYPE-DISTANCE: ", hroute->totalWeight());
#		endif

		m_cache->remember(from, to, hroute);
		return hroute;
	} catch(const NoApplicableConversionException &exc)
	{
		m_cache->remember(from, to,  Cache::IMPOSSIBLE);
#		ifndef NDEBUG
			dbg::trace << "@TYPE-DISTANCE: IMPOSSIBLE" << dbg::endl;
#		endif
		throw;
	}
}

Handle<ConversionTree> ConversionTable::generateConversionTree(const RobinType &source,
												const RobinType *stopType,
												bool constConversionTree) const
{

	/* Use Dijkstra's shortest-path algorithm (modified).
	 * Refer to Cormen's "Introduction to Algorithms", sub-topic 25.2
	 * for details */
	AssocHeap<Conversion::Weight, const RobinType *> bestWeightHeap;
	Handle<ConversionTree> previousStepMap(new ConversionTree(source));
	TypeToWeightMap  distanceMap;
										  // 'd' maps d[u] if d[u] < INFINITE.

	distanceMap[&source] = Conversion::Weight::ZERO;
	bestWeightHeap.insert(Conversion::Weight::ZERO, &source);

	/* While bestWeightHeap not empty */
	while (bestWeightHeap.size() != 0) {
		/* u <- Extract-Min(bestWeightHeap) */
		const RobinType *u;
		Conversion::Weight reachedWeight;
		bestWeightHeap.extractMinimum(reachedWeight, u);

		if(0 != stopType && u->getID() == stopType->getID())
		{
			break;
		}

		u->proposeConversionContinuations(reachedWeight,bestWeightHeap,constConversionTree,distanceMap,*previousStepMap);
	}
	return previousStepMap;
}



/**
 * Flushes internal caches, so the next time a conversion is requested,
 * the route is recomputed.
 */
void ConversionTable::forceRecompute()
{
	m_cache->flush();
}





/**
 */
NoApplicableConversionException::NoApplicableConversionException
(const RobinType *fr, const RobinType *t)
	: from(fr), to(t) { }

/**
 */
const char *NoApplicableConversionException::what() const throw()
{
	return "no conversion route connects source and destination types.";
}


/**
 * Reimplement getInstance(). This is a workaround for a malicious
 * bug in some compilers.
 */
ConversionTable *ConversionTableSingleton::getInstance()
{
	return Pattern::Singleton<ConversionTable>::getInstance();
}

namespace {
	/**
	 * A dummy class.
	 * When the library is unloaded, deletes the instance of the conversion
	 * table.
	 */
	class ConversionTableDeallocator
	{
	public:
		~ConversionTableDeallocator() {
			delete ConversionTableSingleton::getInstance();
		}
	} _ctdealloc;
}


} // end of namespace Robin
