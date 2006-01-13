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

#if defined(WITH_STD_HASHMAP)
#include <ext/hash_map>
#elif defined(WITH_EXT_HASHMAP)
#include <ext/hash_map>
namespace std {
	using __gnu_cxx::hash_map;
}
#else
#include <map>
#endif

#ifndef PATTERN_CACHE_H
#define PATTERN_CACHE_H

namespace Pattern {



/*
 * Caches help maintaining results of previous computations, to refrain
 * from doing them over and over again. When computations take place, the
 * cache stores the results (using Cache::remember). When a processing
 * is requested later, the result can be fetched (using Cache::recall).
 */
template < class Key, class Result, class HashTraits >
class Cache
{
private:
	typedef typename HashTraits::KeyHashFunctor KeyHashFunctor;
	typedef typename HashTraits::KeyIdentityFunctor KeyIdentityFunctor;
	typedef typename HashTraits::KeyCompareFunctor KeyCompareFunctor;

#ifdef WITH_EXT_HASHMAP
	typedef std::hash_map<Key, Result, 
					 KeyHashFunctor, KeyIdentityFunctor> cache_map;
#else
	typedef std::map<Key, Result, KeyCompareFunctor> cache_map;
#endif

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
	inline Result recall(const Key& key) const
	{
		typename cache_map::const_iterator slot = latest.find(key);

		if (slot == latest.end()) {
#ifdef HARD_PROFILE
			++misses;
#endif
			return HashTraits::MISSED;
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
			HashTraits::dispose((ci++)->first);
		latest.clear();
	}

private:
	const size_t m_max_keep;
	static const size_t DEFAULT_MAX_KEEP = 1206;
};

#ifdef HARD_PROFILE
template < class Key, class Result, class HashTraits >
int Cache<Key, Result, HashTraits>::hits = 0;
template < class Key, class Result, class HashTraits >
int Cache<Key, Result, HashTraits>::misses = 0;
#endif

} // end of namespace Pattern

#endif
