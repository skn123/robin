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
#include "class.h"
#include "enumeratedtype.h"
#include "overloadedset.h"
#include "fundamental_conversions.h"
#include "intrinsic_type_arguments.h"

#include "../debug/trace.h"


namespace Robin {


/**
 * Remembers latest routes that were calculated during the existance of
 * this table, so that they can be used later.
 */
class ConversionTable::Cache
{
public:
	struct Key { 
		const TypeOfArgument *pSource; 
		const TypeOfArgument *pTarget;
		void *insight;
	};

	class KeyHashFunctor
	{
	public:
		size_t operator()(const Key& key) const {
			return (size_t)key.pSource + (size_t)key.pTarget + 
				(size_t)key.insight; 
		}
	};

	class KeyIdentityFunctor
	{
	public:
		bool operator()(const ConversionTable::Cache::Key& key1,
						const ConversionTable::Cache::Key& key2) const
		{
			return (key1.pSource == key2.pSource) &&
			       (key1.pTarget == key2.pTarget) &&
			       (key1.insight == key2.insight);
		}
	};

	class KeyCompareFunctor
	{
	public:
		bool operator()(const Key& key1, const Key& key2) const
		{
			return (key1.pSource < key2.pSource) ||
				( (key1.pSource == key2.pSource) &&
				  (key1.pTarget < key2.pTarget) ) ||
				( (key1.pSource == key2.pSource) && 
				  (key1.pTarget == key2.pTarget) && 
				  (key1.insight < key2.insight) );
		}
	};

	typedef Pattern::Cache<Key, Handle<ConversionRoute>, Cache> Internal;
	Internal actual_cache;

	/**
	 * Store an entry in the cache.
	 */
	inline void remember(const TypeOfArgument& source, 
						 const TypeOfArgument& target,
						 Insight insight,
						 Handle<ConversionRoute> chosenRoute)
	{
		const Key key = { &source, &target, insight.i_ptr };
		actual_cache.remember(key, chosenRoute);
	}

	/**
	 * Fetch an entry from the cache.
	 */
	inline Handle<ConversionRoute> recall(const TypeOfArgument& source, 
										  const TypeOfArgument& target,
										  Insight insight) const
	{
		const Key key = { &source, &target, insight.i_ptr };
		return actual_cache.recall(key);
	}


	inline void flush() { actual_cache.flush(); }

	static inline void dispose(const Key& key) { }


	static const Handle<ConversionRoute> IMPOSSIBLE;
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
	dbg::trace << "// @REGISTER: conversion from "
			   << &*(edge->sourceType()) << " to " << &*(edge->targetType())
			   << dbg::endl;
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
ConversionTable::getAdjacentTo(const TypeOfArgument& node) const
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
ConversionTable::getEdgeConversion(const TypeOfArgument& node) const
{
	exitmap::const_iterator existing = m_edgeconv.find(&node);

	if (existing != m_edgeconv.end()) {
		return existing->second;
	}
	else {
		return Handle<Conversion>();
	}
}


/**
 * Returns the best path between two types, along edges
 * which refer to available conversions, in terms of conversion
 * weights (Conversion::Weight).
 *
 * @param from source type
 * @param to target type
 */
Handle<ConversionRoute> 
ConversionTable::bestSingleRoute(const TypeOfArgument& from,
								 const TypeOfArgument& to) const
{
	Insight insight;
	insight.i_long = 0;
	return bestSingleRoute(from, to, insight);
}

/**
 * Returns the best path between two types, along edges
 * which refer to available conversions, in terms of conversion
 * weights (Conversion::Weight).
 *
 * @param from source type
 * @param to target type
 * @param insight some information about the *value* which is being converted
 */
Handle<ConversionRoute> 
ConversionTable::bestSingleRoute(const TypeOfArgument& from,
								 const TypeOfArgument& to,
								 Insight insight) const
{
	/* - first we might have some luck and the requested route may be in
	 *   the cache */
	Handle<ConversionRoute> hcached;
	if (hcached = m_cache->recall(from, to, insight))
		if (hcached == Cache::IMPOSSIBLE)
			throw NoApplicableConversionException(&from, &to);
		else
			return hcached;

	/* Use Dijkstra's shortest-path algorithm (modified).
	 * Refer to Cormen's "Introduction to Algorithms", sub-topic 25.2
	 * for details */
	AssocHeap<Conversion::Weight, const TypeOfArgument *> Q;
	std::map<const TypeOfArgument *, Handle<Conversion> > pred;
	std::map<const TypeOfArgument *, Conversion::Weight>  d;
	                                      // 'd' maps d[u] if d[u] < INFINITE.

	dbg::trace << "// trying to go from " << &from << " to " << &to 
			   << dbg::endl;

	/* Initialize-Single-Source(G, s) */
	for (adjmap::const_iterator node_i = m_graph.begin();
		 node_i != m_graph.end(); ++node_i) {
		Q.insert(Conversion::Weight::INFINITE, node_i->first);
	}
	if (Q.containsElement(&from))
		Q.decreaseKey(&from, Conversion::Weight::ZERO);
	else
		Q.insert(Conversion::Weight::ZERO, &from);
	/* While Q not empty */
	while (Q.size() != 0) {
		/* u <- Extract-Min(Q) */
		const TypeOfArgument *u;
		Conversion::Weight du;
		Q.extractMinimum(du, u);
		d[u] = du;
		/* for each vertex v in Adj[u] */
		AdjacencyList adj_u = getAdjacentTo(*u);
		addTrivialConversions(adj_u, u);
		for (AdjacencyList::const_iterator v_i = adj_u.begin();
			 v_i != adj_u.end(); ++v_i) {
			const TypeOfArgument *v = &*(v_i->targetNode);
			/* do Relax(u, v, w) */
			Conversion::Weight dv;
			dv = (d.find(v) != d.end()) ? d.find(v)->second 
				                        : Conversion::Weight::INFINITE;

			Conversion::Weight w = (u == &from) ? v_i->edge->weight(insight)
				                                : v_i->edge->weight();

			if (du + w < dv) {
				pred[v] = v_i->edge /* (u, v) */;
				d[v] = dv = du + w;
				// - adjust Q to reflect changes in key
				if (Q.containsElement(v))		Q.decreaseKey(v, dv);
				                     else		Q.insert(dv, v);
			}
		}
	}

	/* Was any path found at all? */
	if (&to != &from && pred.find(&to) == pred.end()) {
		dbg::trace << "// @TYPE-DISTANCE: infinite" << dbg::endl;
		m_cache->remember(from, to, insight, Cache::IMPOSSIBLE);
		throw NoApplicableConversionException(&from, &to);
	}

	/* Reconstruct the path from the source to the destination */
	Handle<ConversionRoute> hroute(new ConversionRoute);
	ConversionRoute& route = *hroute;

	for (const TypeOfArgument *tail = &to; tail != &from; 
		 tail = &*(route[0]->sourceType())) {
		/* Append edge (pi[tail], tail) at beginning of path */
		route.insert(route.begin(), pred[tail]);
	}

#ifndef NDEBUG
	dbg::trace("@TYPE-DISTANCE: ", route.totalWeight(insight));
#endif

	m_cache->remember(from, to, insight, hroute);
	return hroute;
}

/**
 * Finds best conversion paths to all the elements in
 * the source list, which end up in corresponding elements in the
 * target list. Elements are independant of each other.
 */
void
ConversionTable::bestSequenceRoute(const std::vector<Handle<TypeOfArgument> >&
								   from_seq,
								   const std::vector<Handle<TypeOfArgument> >&
								   to_seq,
								   std::vector<Handle<ConversionRoute> >&
								   result_seq) const
{
	std::vector<Handle<TypeOfArgument> >::const_iterator from_it, to_it;
	std::vector<Handle<ConversionRoute> >::iterator res_it;

	result_seq.resize(std::min(from_seq.size(), to_seq.size()));
	res_it = result_seq.begin();

	for (from_it = from_seq.begin(), to_it = to_seq.begin();
		 from_it != from_seq.end() && to_it != to_seq.end();
		 ++from_it, ++to_it) {
		/* Calculate each 'best' set of conversions */
		*(res_it++) = bestSingleRoute(**from_it, **to_it);
	}
}

/**
 * Finds best conversion paths to all the elements in
 * the source list, which end up in corresponding elements in the
 * target list. Elements are independant of each other.
 */
void
ConversionTable::bestSequenceRoute(const std::vector<Handle<TypeOfArgument> >&
								   from_seq,
								   const std::vector<Insight>& insight_seq,
								   const std::vector<Handle<TypeOfArgument> >&
								   to_seq,
								   std::vector<Handle<ConversionRoute> >&
								   result_seq) const
{
	std::vector<Handle<TypeOfArgument> >::const_iterator from_it, to_it;
	std::vector<Insight>::const_iterator insight_it;
	std::vector<Handle<ConversionRoute> >::iterator res_it;

	result_seq.resize(std::min(from_seq.size(), to_seq.size()));
	res_it = result_seq.begin();

	for (from_it = from_seq.begin(), to_it = to_seq.begin(), 
			 insight_it = insight_seq.begin();
		 from_it != from_seq.end() && to_it != to_seq.end() &&
			 insight_it != insight_seq.end();
		 ++from_it, ++to_it, ++insight_it) {
		/* Calculate each 'best' set of conversions */
		*(res_it++) = bestSingleRoute(**from_it, **to_it, *insight_it);
	}

	assert(from_it == from_seq.end());
	assert(to_it == to_seq.end());
	assert(insight_it == insight_seq.end());
}

/**
 * Finds best conversion paths to all the elements in
 * the source list, which end up in corresponding elements in the
 * target list. Elements are independant of each other.
 * (this is an optimization version using less vector that the previous
 * one).
 */
void ConversionTable::
bestSequenceRoute(Handle<TypeOfArgument> from_seq[],
				  const std::vector<Handle<TypeOfArgument> >& to_seq,
				  Handle<ConversionRoute> result_seq[]) const
{
	std::vector<Handle<TypeOfArgument> >::const_iterator to_it;
	Handle<TypeOfArgument> *from_it;
	Handle<ConversionRoute> *res_it;

	for (from_it = from_seq, to_it = to_seq.begin(), res_it = result_seq;
		 to_it != to_seq.end(); ++from_it, ++to_it) {
		/* Calculate each 'best' set of conversions */
		*(res_it++) = bestSingleRoute(**from_it, **to_it);
	}
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
 * Adds trivial conversions to the given adjacency list.
 */
void ConversionTable::addTrivialConversions(
		AdjacencyList &list, const TypeOfArgument *from) const
{
	Adjacency adj;
	adj.targetNode = ArgumentScriptingElement;
	adj.edge = Handle<Conversion>(new TrivialConversion);
	if (m_graph.find(from) != m_graph.end())
		adj.edge->setSourceType(
				m_graph.find(from)->second.begin()->edge->sourceType());
	adj.edge->setTargetType(adj.targetNode);
	list.push_back(adj);
}


/**
 */
NoApplicableConversionException::NoApplicableConversionException
(const TypeOfArgument *fr, const TypeOfArgument *t)
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
