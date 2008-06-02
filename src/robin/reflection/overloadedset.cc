// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/overloadedset.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "overloadedset.h"

#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <assert.h>
#include <pattern/cache.h>
#include <pattern/singleton.h>

#include "conversion.h"
#include "conversiontable.h"
#include "memorymanager.h"
#include "../frontends/framework.h"

#include "../debug/trace.h"

#   define msize nargs


namespace Robin {

enum overloading_relationship {
	OL_BETTER,
	OL_WORSE,
	OL_EQUIVALENT,
	OL_AMBIGUOUS 
};

typedef std::vector<Handle<ConversionRoute> > ListConversion;
typedef std::vector<Conversion::Weight>       WeightList;
typedef std::vector<Insight>                  Insights;
typedef std::vector<Conversion::Weight>       WeightList;
typedef std::vector<Handle<TypeOfArgument> > TypeOfArguments;



const char *OverloadingNoMatchException::what() const throw() {
	return "no overloaded member matches arguments.";
}

const char *OverloadingAmbiguityException::what() const throw() {
	return "call is ambiguous with given arguments.";
}

const char *ArgumentArrayLimitExceededException::what() const throw() {
	return "Robin::OverloadedSet: argument limit exceeded.";
}

#ifdef IS_ARGUMENT_LIMIT
const int OverloadedSet::ARGUMENT_ARRAY_LIMIT = 12;
#endif

#define I Conversion::Weight::INFINITE /* nasty shortcut */

/**
 * Remembers latest routes that were calculated during the existance of
 * this table, so that they can be used later.
 */
class OverloadedSet::Cache
{
public:
	struct Key {
		const OverloadedSet *group;
		size_t nargs;
		const Handle<TypeOfArgument> *args;
		const Insight* insights;
	};

    struct Value {
        size_t                                  alt_index;
        std::vector<Handle<ConversionRoute> >   lightroutes;
    };

	class KeyHashFunctor
	{
	public:
		size_t operator()(const Key& key) const
		{
			size_t accum = (size_t)key.group;
			for (size_t i = 0; i < key.nargs; ++i)
				accum += (size_t)&*(key.args[i]);
			return accum;
		}
	};

	class KeyIdentityFunctor
	{
	public:
		bool operator()(const Key& key1,
						const Key& key2) const
		{
			if (key1.group != key2.group) return false;
			if (key1.nargs != key2.nargs) return false;
			for (size_t i = 0; i < key1.nargs; ++i) {
				if (&*(key1.args[i]) != &*(key2.args[i])) return false;
				if (key1.insights[i] != key2.insights[i]) return false;
			}
			return true;
		}
	};

	class KeyCompareFunctor
	{
	public:
		bool operator()(const Key& key1,
						const Key& key2) const
		{
			if (key1.group < key2.group) return true;
			if (key1.group > key2.group) return false;
			if (key1.nargs < key2.nargs) return true;
			if (key1.nargs > key2.nargs) return false;
			for (size_t i = 0; i < key1.nargs; ++i) {
				if (&*(key1.args[i]) < &*(key2.args[i])) return true;
				if (&*(key1.args[i]) > &*(key2.args[i])) return false;
				if (key1.insights[i] < key2.insights[i]) return true;
				if (key1.insights[i] > key2.insights[i]) return false;
			}
			return false;
		}
	};

	typedef Pattern::Cache<Key, Value, OverloadedSet::Cache> Internal;
	Internal actual_cache;

	/**
	 * Store an entry in the cache.
	 */
	inline void remember(const OverloadedSet *me,
						 const size_t nargs, const Handle<TypeOfArgument> args[],
						 const Insight insights[],
						 size_t chosenAlternative,
                         const std::vector<Handle<ConversionRoute> >& convs)
	{
		Handle<TypeOfArgument> *args_clone = cloneArgsArray(nargs, args);
		Insight *insights_clone = cloneInsightsArray(nargs, insights);

		const Key key = { me, nargs, args_clone, insights_clone };
        const Value value = { chosenAlternative, convs };
		actual_cache.remember(key, value);
	}

	/**
	 * Fetch an entry in the cache.
	 */
	inline const Value& recall(const OverloadedSet *me, 
                               size_t nargs,
                               const Handle<TypeOfArgument> args[],
                               const Insight insights[])
	{
		const Key key = { me, nargs, args, insights };
		return actual_cache.recall(key);
	}

	/**
	 * Relinquish cache contents.
	 */
	inline void flush() { actual_cache.flush(); }

	/// Needed for memory management. Cannot be private because Pattern::Cache
	/// has to call it.
	static inline void dispose(const Key& key) 
	{ 
		delete[] key.args; delete[] key.insights; 
	}

	static const unsigned int IMPOSSIBLE;
	static const Value MISSED;

private:
	/**
	 * Copy args array for store in cache key.
	 */
	Handle<TypeOfArgument>
		*cloneArgsArray(int nargs, const Handle<TypeOfArgument> args[]) 
	{
		Handle<TypeOfArgument> *args_clone = new Handle<TypeOfArgument>[nargs];
		for (int i = 0; i < nargs; ++i) args_clone[i] = args[i];
		return args_clone;
	}

	/**
	 * Copy insights array for store in cache key.
	 */
	Insight *cloneInsightsArray(int nargs, const Insight insights[]) 
	{
		Insight *insights_clone = new Insight[nargs];
		for (int i = 0; i < nargs; ++i) insights_clone[i] = insights[i];
		return insights_clone;
	}
};

const unsigned int OverloadedSet::Cache::IMPOSSIBLE = (unsigned int)-1;
const OverloadedSet::Cache::Value OverloadedSet::Cache::MISSED = { (unsigned int)-9 };


class OverloadedSet::CacheSingleton 
	: public Pattern::Singleton<OverloadedSet::Cache>
{
};


namespace {

typedef std::map<std::string, size_t> ArgumentIndices;


/**
 * Takes a KeywordArgumentMap and produces a map of its keys in sorted order
 * This relies on the invariant that keys in an std::map are iterated on in sorted order
 */
Handle<ArgumentIndices> arrangeKWArguments(const KeywordArgumentMap& kwargs) {

        Handle<ArgumentIndices> indices(new ArgumentIndices);
       
        int index = 0;
        dbg::trace << "// @ARRANGE_KW_ARGUMENTS: asked to arrange " << kwargs.size() << " kwargs" << dbg::endl;
        for(KeywordArgumentMap::const_iterator i = kwargs.begin(); i != kwargs.end();
                                               ++i, ++index)
        {
                dbg::trace << "//\t---- arg '" << i->first << "' is at index " << index << dbg::endl;
                (*indices)[i->first] = index;
        }

        return indices;

}

/**
 * Produces a mapping in order of argument appearance in the vector
 */
Handle<ArgumentIndices> arrangeKWArgumentsFromFunction(const std::vector<std::string> &arg_names, size_t firstKwarg) {
        Handle<ArgumentIndices> indices(new ArgumentIndices);

        dbg::trace << "// @ARRANGE_KW_ARGUMENTS_FROM_FUNCTION: Total of " << arg_names.size() << " arguments, first kwarg at " << firstKwarg << dbg::endl;

        for(size_t i = firstKwarg; i<arg_names.size(); i++) {
                dbg::trace << "//\tArg '" << arg_names[i] << "' goes to index " << i - firstKwarg << dbg::endl;
                (*indices)[arg_names[i]] = i - firstKwarg;
        }
        return indices;
}

/**
 * Takes an ActualArgumentList <b>list</b>, a KeywordArgumentMap <b>map</b> and an order
 * for kwargs, and produces an ordered argument list consisting of args, followed by arranged kwargs
 */
Handle<ActualArgumentList> orderArguments(const ActualArgumentList& args, 
                                          const KeywordArgumentMap& kwargs,
                                          Handle<ArgumentIndices> indices)
{
        dbg::trace << "// @ORDER_ARGUMENTS: starting" << dbg::endl;
        Handle<ActualArgumentList> ordered_args(new ActualArgumentList(args.size() + kwargs.size()));

        dbg::trace << "// @ORDER_ARGUMENTS: copying " << args.size() << " arguments as-is" << dbg::endl;
        size_t i;
        for(i = 0; i < args.size(); i++) 
        {
            (*ordered_args)[i] = args[i];
        }
        
        for(ArgumentIndices::const_iterator ii = indices->begin(); ii != indices->end(); ii++)
        {
            (*ordered_args)[i + ii->second] = (kwargs.find(ii->first))->second;
            dbg::trace << "//\targument at index " << i + ii->second << " is named '" << ii->first << "'" << dbg::endl;
        }

        return ordered_args;
}

/**
 * Reorders an list given the names for every list element (in order of their appearance in the list)
 * and mapping of names to final indices
 * First n - kwargIndices.size() arguments are simply copied
 */

template <class T>
Handle< T > reorderArguments(const T& args, 
                             const std::vector<std::string>& argumentNames, 
                             Handle<ArgumentIndices> kwargIndices)
{
    dbg::trace << "// @REORDER_ARGUMENTS: Total of " << args.size() << " arguments, " << kwargIndices->size() << " will be reordered" << dbg::endl;
    Handle< T > reordered_list(new T(args.size()));

    size_t numNonKwargs = args.size() - kwargIndices->size();
    dbg::trace << "// @REORDER_ARGUMENTS: Copying " << numNonKwargs << " non-kw arguments" << dbg::endl;
    for(size_t i=0; i < numNonKwargs; i++) {
            (*reordered_list)[i] = args[i];
    }

    for(size_t i= numNonKwargs; i<args.size(); i++) {
            dbg::trace << "\t- The argument '" << argumentNames[i] << "'" << dbg::endl;
            dbg::trace << "\t\tappears at index " << (*kwargIndices)[argumentNames[i]] + numNonKwargs << dbg::endl;
            dbg::trace << "\t\tand will be now at index " << i << dbg::endl;
            (*reordered_list)[i] = args[(*kwargIndices)[argumentNames[i]] + numNonKwargs];
    }

    return reordered_list;

}
/**
 * Compares a pair of sequence conversion suggestions
 * to determine which of them is cheaper. The formal arguments should
 * have had the same type, but implementation issues dictate that
 * one of them is represented by a series of ConversionRoutes, while
 * the other - plainly by a list of weights.
 * <p>The return value is:
 * <ul>
 *  <li>OL_BETTER - if 'suggested' is better (smaller) than 'known';</li>
 *  <li>OL_WORSE - if 'suggested' is worse (greater) than 'known';</li>
 *  <li>OL_EQUIVALENT - if all the weights are the same;</li>
 *  <li>OL_AMBIGIOUS - if there is no way to decide either way.</li>
 * </ul>
 * </p>
 * (vector version)
 */
overloading_relationship compareAlternatives(const WeightList& known,
											 const ListConversion& suggested,
											 const Insights& insights)
{
	WeightList::const_iterator known_iter;
	ListConversion::const_iterator suggested_iter;
	Insights::const_iterator insight_iter;

	assert(known.size() == suggested.size());

	bool worse_witness = false;   /* did we encounter a location which
								   * indicates that 'known' is better? */
	bool better_witness = false;  /* did we encounter a location which
								   * indicates that 'suggested' is better? */

	if (known.size() == 0) return OL_BETTER;  /* optimization */

	/* Go through both vectors in parallel and search for witnesses for
	 * either side */
	for (known_iter = known.begin(), suggested_iter = suggested.begin(),
			 insight_iter = insights.begin();
		 known_iter != known.end() && suggested_iter != suggested.end();
		 ++known_iter, ++suggested_iter, ++insight_iter) {
		/* Compare the two corresponding elements of the two vectors */
		Conversion::Weight suggested_weight = 
			(*suggested_iter)->totalWeight(*insight_iter);
		dbg::trace("    - known:     ", *known_iter);
		dbg::trace("    - suggested: ", suggested_weight);
		if (*known_iter < suggested_weight)
			worse_witness = true;
		else if (suggested_weight < *known_iter)
			better_witness = true;
	}

	if (worse_witness != better_witness)
		return better_witness ? OL_BETTER : OL_WORSE;
	else
		return better_witness ? OL_AMBIGUOUS : OL_EQUIVALENT;
}

/**
 * Compares a pair of sequence conversion suggestions
 * to determine which of them is cheaper. The formal arguments should
 * have had the same type, but implementation issues dictate that
 * one of them is represented by a series of ConversionRoutes, while
 * the other - plainly by a list of weights.
 * <p>The return value is:
 * <ul>
 *  <li>OL_BETTER - if 'suggested' is better (smaller) than 'known';</li>
 *  <li>OL_WORSE - if 'suggested' is worse (greater) than 'known';</li>
 *  <li>OL_EQUIVALENT - if all the weights are the same;</li>
 *  <li>OL_AMBIGIOUS - if there is no way to decide either way.</li>
 * </ul>
 * </p>
 * (array version)
 */
overloading_relationship compareAlternatives(size_t nargs,
											 Conversion::Weight known[],
											 Handle<ConversionRoute>
											    suggested[],
											 Insight insights[])
{
	bool worse_witness = false;   /* did we encounter a location which
								   * indicates that 'known' is better? */
	bool better_witness = false;  /* did we encounter a location which
								   * indicates that 'suggested' is better? */

	if (nargs == 0) return OL_BETTER;  /* optimization */

	/* Go through both vectors in parallel and search for witnesses for
	 * either side */
	for (size_t index = 0; index < nargs; ++index) {
		/* Compare the two corresponding elements of the two vectors */
		Conversion::Weight suggested_weight = 
			suggested[index]->totalWeight(insights[index]);
		if (known[index] < suggested_weight)
			worse_witness = true;
		else if (suggested_weight < known[index])
			better_witness = true;
	}

	if (worse_witness != better_witness)
		return better_witness ? OL_BETTER : OL_WORSE;
	else
		return better_witness ? OL_AMBIGUOUS : OL_EQUIVALENT;
}


/**
 * Checks whether the parameters of two alternatives are absolutely identical.
 * This situation may occur if, for example, a method has a const and a
 * non-const version with the same arguments.
 */
bool identicalAlternatives(Handle<CFunction> alt1, Handle<CFunction> alt2)
{
	const FormalArgumentList& args1 = alt1->signature();
	const FormalArgumentList& args2 = alt2->signature();

	// Compare lengths
	if (args1.size() != args2.size()) return false;

	for (FormalArgumentList::const_iterator arg1_iter = args1.begin(),
			 arg2_iter = args2.begin();
		 arg1_iter != args1.end(); ++arg1_iter, ++arg2_iter) {
		// Compare args
		if (*arg1_iter != *arg2_iter) return false;
	}

	return true;
}

/**
 * Gets the weights of ConversionRoutes from a vector
 * of such. (vector version)
 */
void rememberWeights(const ListConversion& list,
					 const Insights& insights,
					 WeightList& weights)
{
	weights.resize(list.size());
	for (size_t i = 0; i < list.size(); ++i) {
		weights[i] = list[i]->totalWeight(insights[i]);
	}
}

/**
 * Gets the weights of ConversionRoutes from an array
 * of such. (array version)
 */
void rememberWeights(size_t nargs,
					 Handle<ConversionRoute> list[],
					 Insight insights[],
					 Conversion::Weight weights[])
{
	for (size_t i = 0; i < nargs; ++i) {
		weights[i] = list[i]->totalWeight(insights[i]);
	}
}

/**
 * (vector version)
 */
bool conversionPossible(const WeightList& weights)
{
	for (WeightList::const_iterator iter = weights.begin();
		 iter != weights.end(); ++iter) {
		/* Is this possible? */
		if (! iter->isPossible())
			return false;   /* definitely not */
	}

	return true; /* Yeea! */
}

/**
 * (array version)
 */
bool conversionPossible(size_t nargs, Conversion::Weight weights[])
{
	for (size_t i = 0; i < nargs; ++i) {
		/* Is this possible? */
		if (! weights[i].isPossible())
			return false;   /* definitely not */
	}

	return true; /* Yeea! */
}

/**
 * Applies a series of conversion routes to a list
 * of actual arguments to produce a secondary list suitable for
 * calling a function directly.
 * (vector version)
 */
void convertSequence(const ActualArgumentList& original,
					 const ListConversion& conversions,
					 ActualArgumentList& converted,
					 GarbageCollection& gc)
{
	assert(original.size() == conversions.size());
	converted.resize(original.size());
	// Apply each conversion route independantly
	for (size_t i = 0; i < original.size(); ++i) {
		converted[i] = conversions[i]->apply(original[i], gc);
	}
}

/**
 * Applies a series of conversion routes to a list
 * of actual arguments to produce a secondary list suitable for
 * calling a function directly.
 * (array version)
 */
void convertSequence(size_t nargs,
					 const ActualArgumentArray original,
					 Handle<ConversionRoute> conversions[],
					 ActualArgumentArray converted,
					 GarbageCollection& gc)
{
	// Apply each conversion route independantly
	for (size_t i = 0; i < nargs; ++i) {
		converted[i] = conversions[i]->apply(original[i], gc);
	}
}

/**
 * Applies a possible edge conversion which is associated with a given
 * type. The parameter 'value' initially contains the original value, and
 * is assigned the new value if a conversion actually takes place.
 */
void applyEdgeConversions(Handle<TypeOfArgument> type, 
						  scripting_element& value)
{
	Handle<Conversion> exit = ConversionTableSingleton::getInstance()
		->getEdgeConversion(*type);

	if (exit) {
		scripting_element new_value = exit->apply(value);
		MemoryManager::release(value);
		value = new_value;
	}
}




} // end of anonymous namespace


/**
 * Default constructor - builds a set with no
 * alternatives. Add alternatives with 
 * <methodref>addAlternative()</methodref>.
 */
OverloadedSet::OverloadedSet()
	: m_allow_edge(true)
{
}

OverloadedSet::~OverloadedSet()
{
}



/**
 * Declares an additional alternative for overloading.
 * Upon call, this alternative will also be considered among the
 * others.<br />
 * This method is meant to be called in the registration phase. Once
 * alternatives have been declared in the OverloadedSet, it cannot
 * be removed.
 */
void OverloadedSet::addAlternative(Handle<CFunction> alt)
{
	m_alternatives.push_back(alt);
}

/**
 * Declares additional alternatives for overloading,
 * by consuming a second OverloadedSet into the current. The other
 * set is <b>not</b> destroyed and can still be used.
 */
void OverloadedSet::addAlternatives(const OverloadedSet& more)
{
	std::copy(more.m_alternatives.begin(), more.m_alternatives.end(),
			  std::back_inserter<altvec>(m_alternatives));
}

/**
 * Determines whether edge conversions will be applied to the return 
 * value of the function before passing it on to the interpreter.
 */
void OverloadedSet::setAllowEdgeConversions(bool allow)
{
	m_allow_edge = allow;
}


/**
 * Calls the overloaded function with the given
 * arguments. The types of the actual arguments are checked against
 * those of the formal arguments declared in each alternative (only
 * those with equal-length parameter lists, of course), and each is
 * given a 'weight'; implicit conversions are considered. Eventually,
 * the light-most alternative receives the call.
 *
 * @note OverloadingMismatchException is thrown if none of the alternatives
 * suggest valid conversions for all arguments. OverloadingAmbiguityException
 * is thrown if the smallest weight is shared among more than one
 * alternative.
 */
scripting_element OverloadedSet::call(const ActualArgumentList& args, 
		const KeywordArgumentMap &kwargs, scripting_element owner) const 
{ 
    const int nargs = args.size() + kwargs.size();
    #   define msize nargs
    
    typedef std::vector<Handle<ConversionRoute> > ConversionRoutes;
    typedef std::vector<Handle<TypeOfArgument> > TypeOfArguments;
    
    Handle<CFunction> match;                 // matched alternative
    Cache            *cache;
    size_t            cached_alt;            // index fetched/stored in cache
    ActualArgumentList converted_args(msize); // arguments for final call
    
    GarbageCollection gc;                    // temporary objects to clean up
    
#ifdef IS_ARGUMENT_LIMIT
    // - we intend to use arrays, make sure the array limit is not exceeded
    if (nargs > ARGUMENT_ARRAY_LIMIT)
        throw ArgumentArrayLimitExceededException();
#endif
    
    // - acquire the cache
    cache = CacheSingleton::getInstance();
    
    // Get the types of the actual parameters using Passive Translation
    TypeOfArguments              actual_types(msize);
    TypeOfArguments::iterator    typei = actual_types.begin();
    Insights                     actual_insights(msize);
    Insights::iterator           insighti = actual_insights.begin();

    Handle<ArgumentIndices> kwargOrder = arrangeKWArguments(kwargs);
    Handle<ActualArgumentList> canonic_args = orderArguments(args, kwargs, kwargOrder); 

    ActualArgumentList::const_iterator argi;
    for (argi = canonic_args->begin(); argi != canonic_args->end();
         ++argi, ++typei, ++insighti) {
        // Get type and insight and put them in the actual_... vectors
        *typei    = FrontendsFramework::activeFrontend()->detectType(*argi);
        *insighti = FrontendsFramework::activeFrontend()->detectInsight(*argi);
    }
    
    // Locate applicable alternative from m_alternatives.
    //   Look in the cache first, and if this call is not there, perform
    //   a complete overload resolution.
    const Cache::Value& cached_entry = 
        cache->recall(this, nargs, (nargs==0)?0:&actual_types[0], 
                      (nargs==0)?0:&actual_insights[0]);
    if (kwargs.size() == 0 && &cached_entry != &Cache::MISSED) {
        // Found cached alternative, use it (optimization)
        dbg::trace << "Got value from cache, running alternative #" 
                      << cached_alt << dbg::endl;
        match = m_alternatives[cached_entry.alt_index];
        convertSequence(*canonic_args, cached_entry.lightroutes,
                        converted_args, gc);
    }
    else {
        // Now find the best sequence-route from actual_types to any of
        // the alternative prototypes
        bool ambiguity_alert = false;
        Handle<CFunction>                     lightest;
        ConversionRoutes                      lightroutes(msize);
        WeightList                            lightweight(nargs, I);
        ConversionRoutes  needed(msize);   // required conversions
    
        for (altvec::const_iterator alt_iter = m_alternatives.begin();
             alt_iter != m_alternatives.end(); ++alt_iter) {
            if ((kwargs.size() != 0) && !isKwargsValid(**alt_iter, args, kwargs)) {
                continue;
            }

            /* Check this one out */
            dbg::trace << "// Trying alternative #"
                          << (alt_iter - m_alternatives.begin()) << dbg::endl;
            if ((*alt_iter)->signature().size() == nargs) {


                try {
                    dbg::trace << "// @CHECKING: alternative at "
							   << &(**alt_iter) << dbg::endl;
   
                    Handle<FormalArgumentList> canonic_signature =
						reorderArguments((*alt_iter)->signature(), 
										 (*alt_iter)->argNames(), kwargOrder);
                    ConversionTableSingleton::getInstance()->
                        bestSequenceRoute(actual_types, actual_insights,
                                          *canonic_signature,
                                          needed);
    
                    /* Compare the set of conversions needed to make this call
                     * with the set needed for the lightest known alternative 
                     * so far */
                    switch (compareAlternatives(lightweight, needed, 
                                                actual_insights)) {
                    case OL_BETTER: 
                        lightest = *alt_iter;
                        cached_alt = alt_iter - m_alternatives.begin();
                        for (size_t argi = 0; argi < nargs; ++argi)
                            lightroutes[argi] = needed[argi];
                        rememberWeights(needed, actual_insights, lightweight);
                        ambiguity_alert = false;
                        dbg::trace << "// better!" << dbg::endl;
                        break;
                    case OL_EQUIVALENT:
                    case OL_AMBIGUOUS:
                        if (!identicalAlternatives(lightest, *alt_iter))
                            ambiguity_alert = true;
                        break;
                    }
                }
                catch (NoApplicableConversionException& ) {
                    dbg::trace << "// impossible!" << dbg::endl;
                }
            }
        }
    
    
        dbg::trace << "// @LIGHTEST: " << &(*lightest) << dbg::endl;
        if (nargs > 0)
            dbg::trace("@LIGHTWEIGHT: ", lightweight[0]);
    
        // Now, if a valid alternative was found, issue the call.
        // Otherwise, issue an exception.
        if (!conversionPossible(lightweight) || !lightest) {
            throw OverloadingNoMatchException();
        }
        else if (ambiguity_alert) {
            throw OverloadingAmbiguityException();
        }
        else {
            if(kwargs.size() == 0) {
               cache->remember(this, nargs, (nargs==0)?0:&actual_types[0], 
			       (nargs==0)?0:&actual_insights[0],
                               cached_alt, lightroutes);
            }
            match = lightest;
            convertSequence(*canonic_args, lightroutes, converted_args, gc);
        }
    }
    
    
    if (match) {
        scripting_element result;
        if (kwargs.size() == 0) {
            result = match->call(converted_args, owner);
        }
        else {
            // reverse the order of canonic arguments to match function call
            std::vector<std::string> canon_arg_names = match->argNames();
            std::sort(canon_arg_names.begin() + args.size(), 
                      canon_arg_names.end());

            Handle<ActualArgumentList> function_order_args =
                reorderArguments(converted_args, 
                                 canon_arg_names, 
                                 arrangeKWArgumentsFromFunction(match->argNames(), 
                                                                args.size()));
            
            result = match->call(*function_order_args, owner);
        }

        if (m_allow_edge)
            applyEdgeConversions(match->returnType(), result);
        gc.cleanUp();
        return result;
    }
    else {
        throw OverloadingNoMatchException();
    }
}


/**
 * Finds an alternative which matches the given
 * prototype exactly (without conversions).
 * <p>The return value is 'null' if no prototype matched.</p>
 */
Handle<CFunction>
OverloadedSet::seekAlternative(const FormalArgumentList& prototype) const
{
	for (altvec::const_iterator alt_iter = m_alternatives.begin();
		 alt_iter != m_alternatives.end(); ++alt_iter) {
		// Look for an exact match
		if ((*alt_iter)->signature() == prototype)
			return (*alt_iter);
	}

	return Handle<CFunction>();
}


bool OverloadedSet::isKwargsValid(const CFunction& cfunc, const ActualArgumentList& args,
                                  const KeywordArgumentMap &kwargs) const 
{
    try {
        cfunc.mergeWithKeywordArguments(args, kwargs);
        return true;
    } catch(InvalidArgumentsException &e) {
        return false;
    }
}


void OverloadedSet::forceRecompute()
{
	CacheSingleton::getInstance()->flush();
}


#ifdef HARD_PROFILE
/**
 * A dummy class.
 * When the library is unloaded, reports the state of the cache.
 */
class OverloadedSet::CacheReporter
{
public:
	~CacheReporter() {
		fprintf(stderr,
				"OverloadedSet cache debugging report:\n"
				"-------------------------------------\n"
				" Hits: %i        Misses: %i\n\n", OverloadedSet::Cache::hits,
				OverloadedSet::Cache::misses);
	}
} _ctdealloc;
#endif

} // end of namespace Robin
