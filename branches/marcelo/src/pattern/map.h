/*
 * hash.h
 *
 *  Created on: Jan 10, 2010
 *      Author: Marcelo Taube
 */

#ifndef PATTERN_MAP_H
#define PATTERN_MAP_H

#	if defined(WITH_STD_HASHMAP)
#		include <ext/hash_map>

#	elif defined(WITH_EXT_HASHMAP)
#		include <ext/hash_map>
#   endif

#include <map>

namespace Pattern {

/**
 * The class 'Map' has the same functionality as the regular std::map
 * but tries to use the fastest available implementation which
 * might be a hash map.
 * It maps between a key type and a value type.
 *
 * The template parameter MapTraits has information needed to generate
 * the map, which is:
 * - MapTraits::Key : the type of the key elements
 * - MapTraits::Value : the type of the value elements
 * - MapTraits::KeyHashFunctor: a class which has a function with the
 *  	signature 'size_t operator()(const Key &key) const' and returns a value
 *      which returns allways the same size_t for a particular key.
 * - MapTraits::KeyIdentityFunctor: a class which has a function with the
 * 		signature 'size_t operator()(const Key &, const Key &) const'
 * 		which returns true iff the two keys are the same.
 * - MapTraits::KeyCompareFunctor: a class which has a function with the
 * 		signatre ' bool operator()(const Key &a, const Key &b) const'
 * 		which defines an order between all the keys.  The order has to
 * 		be correctly defined meaning that it should hold transitivity,
 * 		antireflexivity, antisimetry.
 *
 * Depending on the compiled used, not all the functions will be used,
 * thus it is recommended testing in several systems after a MapTraits
 * was modified.
 */
template < typename MapTraits > class Map;

/**
 * Tree map is like Map but it is always implemented by std::map.
 */
template < typename MapTraits >
class TreeMap : public std::map<typename MapTraits::Key,
								typename MapTraits::Value,
								typename MapTraits::KeyCompareFunctor>
{

};

/**
 * Tree multi map is like MultiMap but it is always implemented by std::mult_map.
 */
template < typename MapTraits >
class TreeMultiMap : public std::multimap<typename MapTraits::Key,
								typename MapTraits::Value,
								typename MapTraits::KeyCompareFunctor>
{

};

#if defined(WITH_STD_HASHMAP)
	template < typename MapTraits >
	class Map : public std::hash_map<typename MapTraits::Key,
									typename MapTraits::Value,
									typename MapTraits::KeyHashFunctor,
									typename MapTraits::KeyIdentityFunctor>
	{

	};


#elif defined(WITH_EXT_HASHMAP)
	template < typename MapTraits >
	class Map : public __gnu_cxx::hash_map<typename MapTraits::Key,
									typename MapTraits::Value,
									typename MapTraits::KeyHashFunctor,
									typename MapTraits::KeyIdentityFunctor>
	{

	};




#else
	template < typename MapTraits >
	class Map : public TreeMap<MapTraits>
	{

	};


#endif

#if defined(WITH_STD_HASHMAP)

template < typename MapTraits >
class MultiMap : public std::hash_multimap<typename MapTraits::Key,
								typename MapTraits::Value,
								typename MapTraits::KeyHashFunctor,
								typename MapTraits::KeyIdentityFunctor>
{
	typedef typename std::hash_multimap::<
		typename MapTraits::Key,
		typename MapTraits::Value,
		typename MapTraits::KeyHashFunctor,
		typename MapTraits::KeyIdentityFunctor
		>::iterator iterator;

#elif defined(WITH_EXT_HASHMAP)

template < typename MapTraits >
class MultiMap : public __gnu_cxx::hash_multimap<typename MapTraits::Key,
								typename MapTraits::Value,
								typename MapTraits::KeyHashFunctor,
								typename MapTraits::KeyIdentityFunctor>
{
	typedef typename __gnu_cxx::hash_multimap<
							typename MapTraits::Key,
							typename MapTraits::Value,
							typename MapTraits::KeyHashFunctor,
							typename MapTraits::KeyIdentityFunctor
							>::iterator iterator;

#else

template < typename MapTraits >
class MultiMap : public TreeMultiMap<MapTraits>
{
	typedef typename TreeMultiMap<MapTraits>::iterator iterator;
#endif
public:
		/**
		 * Type returned by method MultiMap::search.
		 * See the documentation of that method.
		 */
		class Results {
		private:
			typename MultiMap::iterator m_it;
			typename MultiMap::iterator m_end;
		public:
			inline void next(){
				m_it++;
			}
			inline bool finished() {
				return m_it==m_end;
			}
			inline typename MapTraits::Value operator *(){
				return m_it->second;
			}
			inline typename MapTraits::Value operator ->(){
				return m_it->second;
			}
			inline Results(typename MultiMap::iterator it, typename MultiMap::iterator end)
				: m_it(it), m_end(end)
			{

			}
		};

		/**
		 *  It provides an easy interface to search in multimaps, clearer
		 *  than the interface of STL
		 *  ( some would say at the cost of being less generic).
		 *
		 *	To search for several values which map to a type use code
		 *	similar to this:
		 *
		 *	MultiMap<MapTraits>::Results results = myMap.search(myKey);
		 *	for(;!results.finished();results.next()) {
		 *		Value myValue = *results;
		 *		// do something with the value
		 *	}
		 */
		inline Results search(typename MapTraits::Key key) {
			std::pair<typename MultiMap::iterator, typename MultiMap::iterator>
				range = equal_range(key);
			return Results(range.first, range.second);
		}

};//end of class MultiMap

} //end of namespace Pattern

#endif // PATTERN_MAP_H
