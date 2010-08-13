// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * handles.h
 *
 * @par TITLE
 * Generic Cache
 */


#ifndef PATTERN_CACHE_H
#define PATTERN_CACHE_H

#include "map.h"

namespace Pattern {



/*
 * Caches help maintaining results of previous computations, to refrain
 * from doing them over and over again. When computations take place, the
 * cache stores the results (using Cache::remember). When a processing
 * is requested later, the result can be fetched (using Cache::recall).
 *
 * CacheTraits is the only template parameter received by the Cache
 * class. It extends the notion of MapTraits from the file pattern/map.h.
 * It is a class that has all the members of MapTraits plus:
 *  - CacheTraits::getMissedElement - returns a member of the type CacheTraits::Value
 *  	which will be returned when the key is not present in the
 *  	cache (of course it might throw an exception too).
 *  	The signature is 'static const Value &CacheTraits::getMissedElement()'
 *  - CacheTraits::dispose - a function of signature
 *  		'static void dispose(const Key& key)'
 *  	which will be called when an element is removed from the cache
 *  	(maybe because there was no space to hold it)
 *  	It is a good place to add resource freeing if the destructor
 *  	of Key does not do it by itself.
 */
template < class CacheTraits >
class Cache
{
private:
	typedef typename CacheTraits::Key Key;
	typedef typename CacheTraits::Value Result;
	typedef Map<CacheTraits> cache_map;
	cache_map latest;

public:
#ifdef HARD_PROFILE
	static int hits;             // - keeps statistics
	static int misses;
#endif

	Cache() : m_max_keep(DEFAULT_MAX_KEEP) { }
	Cache(size_t max_keep) : m_max_keep(max_keep) { }

	/**
	 * Store an entry in the cache.
	 */
	void remember(const Key& key, Result result)
	{
		if (latest.size() >= m_max_keep) flush();
		latest[key] = result;
	}

	/**
	 * Fetch an entry from the cache.
	 */
	inline const Result& recall(const Key& key) const
	{
		typename cache_map::const_iterator slot = latest.find(key);

		if (slot == latest.end()) {
#ifdef HARD_PROFILE
			++misses;
#endif
			return CacheTraits::getMissedElement();
		}
		else {
#ifdef HARD_PROFILE
			++hits;
#endif
			return slot->second;
		}
	}

	/**
	 * Clear table.
	 */
	void flush()
	{
		for (typename cache_map::iterator ci = latest.begin(); 
			 ci != latest.end(); )
			CacheTraits::dispose((ci++)->first);
		latest.clear();
	}

private:
	const size_t m_max_keep;
	static const size_t DEFAULT_MAX_KEEP = 1206;
};

#ifdef HARD_PROFILE
template < class CacheTraits >
int Cache<CacheTraits>::hits = 0;
template < class CacheTraits >
int Cache<CacheTraits>::misses = 0;
#endif

} // end of namespace Pattern

#endif
