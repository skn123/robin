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
#include <sstream>
#include <algorithm>
#include <robin/debug/assert.h>
#include <pattern/cache.h>
#include <pattern/singleton.h>

#include "callresolution.h"
#include "conversion.h"
#include "conversiontable.h"
#include "memorymanager.h"
#include "../frontends/framework.h"
#include "preresolveoverloadedset.h"
#include "../debug/trace.h"



namespace Robin {



const char *OverloadingNoMatchException::what() const throw() {
	return m_message.c_str();
}

OverloadingNoMatchException::OverloadingNoMatchException(const Robin::OverloadedSet &overloadedmethod, const Robin::CallRequest &req) throw()
{
	std::ostringstream err;

	err << "OverloadingNoMatchException: "
			"No overloaded member matches arguments. Will print signatures and passed args. "
			"For conversion costs please use the 'robin.weighConversion' function.\n"
			"Parameters are: ";
	err << req;
	err << "\n"
		   "Candidates are:\n";
	overloadedmethod.printCandidates(err);
	m_message = err.str();
}

OverloadingNoMatchException::~OverloadingNoMatchException() throw()
{

}

OverloadingAmbiguityException::OverloadingAmbiguityException(const Robin::OverloadedSet &overloadedmethod, const OverloadedSet::altlist &competingAlternatives) throw()
{
	std::ostringstream err;

	err << "OverloadingAmbiguityException:\n"
		   "There is no winner between all the possible signatures of the functions.\n"
		   "The competing signatures are:\n";

	OverloadedSet::printCandidates(err, competingAlternatives);
	m_message = err.str();
}

OverloadingAmbiguityException::~OverloadingAmbiguityException() throw()
{

}

const char *OverloadingAmbiguityException::what() const throw() {
	return m_message.c_str();
}

const char *ArgumentArrayLimitExceededException::what() const throw() {
	return "Robin::OverloadedSet: argument limit exceeded.";
}

#ifdef IS_ARGUMENT_LIMIT
const size_t OverloadedSet::ARGUMENT_ARRAY_LIMIT = 12;
#endif

namespace {


Handle<ArgumentIndices> arrangeKWArgumentsFromFunction(const std::vector<std::string> &arg_names, size_t firstKwarg);
void convertSequence(const ActualArgumentList& original, const ConversionRoutes& conversions, ActualArgumentList& converted, GarbageCollection& gc);
void applyEdgeConversions(Handle<RobinType> type, scripting_element& value);

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
            dbg::trace << "\t\tappears at index " << i << dbg::endl;
            dbg::trace << "\t\tand will be now at index " << (*kwargIndices)[argumentNames[i]] + numNonKwargs << dbg::endl;
            (*reordered_list)[(*kwargIndices)[argumentNames[i]] + numNonKwargs] = args[i];
    }
    return reordered_list;

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


} //


OverloadedSet::CallResolution::CallResolution( Handle<CallRequest> &req,
							const Handle<CFunction> &calledFunc)
	: m_function(calledFunc), m_request(req), m_weights(new WeightList)
{

	if(req->m_nargsPassedByName > 0) {
	m_reordered_signature =
		reorderArguments(calledFunc->signature(),
						 calledFunc->argNames(),
						 req->m_kwargOrder);

	m_reordered_signature_names =
			reorderArguments(calledFunc->argNames(),
							 calledFunc->argNames(),
							 req->m_kwargOrder);
	} else {
		m_reordered_signature = Handle<RobinTypes>(new RobinTypes(calledFunc->signature()));
		m_reordered_signature_names = Handle<std::vector<std::string> >(new std::vector<std::string>(calledFunc->argNames()));
	}


	m_routes.resize(req->m_nargs);

	m_weights->resize(req->m_nargs);
	ConversionRoutes::iterator          res_it = m_routes.begin(); // An iterator over all the result conversions
	WeightList::iterator			   weight_it = m_weights->begin();
	RobinTypes::const_iterator from_it = req->m_types.begin(); // An iterator over all the passed types
	RobinTypes::const_iterator from_end = req->m_types.end();
	RobinTypes::const_iterator   to_it = m_reordered_signature->begin(); // An iterator over all the formal types of the function
	RobinTypes::const_iterator   to_end = m_reordered_signature->end();


	ConversionTable *conversionTable  = ConversionTableSingleton::getInstance();

	for (	int i =0;
		 from_it != from_end && to_it != to_end;
		 ++from_it, ++to_it,  ++weight_it, ++res_it, ++i) {
		/* Calculate each 'best' set of conversions */
		dbg::trace << "parameter #" << i << dbg::endl;
		*res_it = conversionTable->bestSingleRoute(**from_it, **to_it);
		*weight_it = (*res_it)->totalWeight();
	}

	if(dbg::trace.on()) {
		Handle<RobinType> retType = calledFunc->returnType();
		if(retType) {
			dbg::trace << "return type: " << *retType << dbg::endl;
		}
	}

	m_allConversionsAreEmpty = m_routes.areAllEmptyConversions();

	assert_true(from_it == from_end);
	assert_true(to_it == to_end);
}


scripting_element OverloadedSet::CallResolution::call(const Handle<ActualArgumentList>& args,
		const KeywordArgumentMap &kwargs, scripting_element owner) const {

	  if (m_function) {
	        GarbageCollection gc;                    // temporary objects to clean up
	        Handle<ActualArgumentList> orderArgumentsReturnValue;
	        /*
	         * canonic_args will have the types passed by name ordered
	         * lexycographically.
	         */
	        const ActualArgumentList *canonic_args;

	        // The keyword arguments will be concatenated to the regular arguments
        	// Also the arguments will be ordered according to the call request order
	        if(kwargs.size() != 0)
	        {
	        	orderArgumentsReturnValue = orderArguments(*args, kwargs, m_request->m_kwargOrder);
	        	canonic_args = &*orderArgumentsReturnValue;
	        } else {
	        	 canonic_args = &*args;
	        }


        	ActualArgumentList converted_args;
	        const ActualArgumentList *passed_args;//will be equal to converted_args except in the
											// case that no conversion is needed.
	        if(!m_allConversionsAreEmpty)
	        {
	        	passed_args = &converted_args;
	        	convertSequence(*canonic_args, m_routes, converted_args, gc);
	        } else {
				passed_args = canonic_args;
	        }

	        scripting_element result;
	        if (kwargs.size() == 0) {
	            result = m_function->call(*passed_args, owner);
	        }
	        else {


	            Handle<ActualArgumentList> function_order_args =
	                reorderArguments(*passed_args,
	                                 *m_reordered_signature_names,
	                                 arrangeKWArgumentsFromFunction(m_function->argNames(),
	                                                                args->size()));

	            result = m_function->call(*function_order_args, owner);
	        }
	        if (m_function->m_allow_edge)
	        {
	        	Handle<RobinType> type = m_function->returnType();
	        	if(type) {
	        		applyEdgeConversions(m_function->returnType(), result);
	        	}
	        }
	        gc.cleanUp();
	        return result;
	    }
	    else {
	        throw OverloadingNoMatchException(*m_request->m_calledFunc,*m_request);
	    }
}

/**
 * Compares the cost of call convertions in this call resolution with the cost
 * in another call resolution, to see which is cheaper.
 * @param proposed the CallResolution to compare with 'this'
 *
 * <p>The return value is:
 * <ul>
 *  <li>OL_PROPOSED_BETTER - if 'proposed' is better (cheap) than 'this';</li>
 *  <li>OL_PROPOSED_WORSE - if 'proposed' is worse (expensive) than 'this';</li>
 *  <li>OL_EQUIVALENT - if all the weights are the same;</li>
 *  <li>OL_AMBIGIOUS - if there is no way to decide either way.
 *  		(each one wins in a different parameter)</li>
 * </ul>
 * </p>
 */
OverloadedSet::CallResolution::overloading_relationship OverloadedSet::CallResolution::compareCost(
											 const OverloadedSet::CallResolution & proposedResolution)
{
	WeightList::const_iterator known_iter;
	WeightList::const_iterator suggested_iter;


	assert_true(proposedResolution.m_weights->size() == m_weights->size());

	bool worse_witness = false;   /* did we encounter a location which
								   * indicates that 'known' is better? */
	bool better_witness = false;  /* did we encounter a location which
								   * indicates that 'suggested' is better? */

	if (m_weights->size() == 0) return OL_PROPOSED_BETTER;  /* optimization */

	int i=0; // A counter to know which parameter are we checking

	/* Go through both vectors in parallel and search for witnesses for
	 * either side */
	for (known_iter = m_weights->begin(), suggested_iter = proposedResolution.m_weights->begin();
		 known_iter != m_weights->end() && suggested_iter != proposedResolution.m_weights->end();
		 ++known_iter, ++suggested_iter, i++)
	{
		dbg::trace << "For parameter " << i << ':' <<  dbg::endl;
		dbg::trace ("    - known:     " , *known_iter);
		dbg::trace ("    - proposed: ", *suggested_iter);
		if (*known_iter < *suggested_iter)
			worse_witness = true;
		else if (*suggested_iter < *known_iter)
			better_witness = true;
	}

	if (worse_witness != better_witness)
		return better_witness ? OL_PROPOSED_BETTER : OL_PROPOSED_WORSE;
	else
		return better_witness ? OL_AMBIGUOUS : OL_EQUIVALENT;
}

/**
 * Remembers latest routes that were calculated during the existance of
 * this table, so that they can be used later.
 */
class OverloadedSet::Cache
{

private:
	/**
		 * CacheTraits is a template parameter for Pattern::Cache
		 */
		struct CacheTraits {
			typedef Handle<CallRequest> Key;
			typedef Handle<OverloadedSet::CallResolution> Value;
			class KeyCompareFunctor
			{
			public:
				bool operator()(const Handle<CallRequest>& key1,
								const Handle<CallRequest>& key2) const
				{
					return *key1 < *key2;
				}
			};

			class KeyIdentityFunctor
			{
			public:
				bool operator()(const Handle<CallRequest>& key1,
								const Handle<CallRequest>& key2) const
				{
					return *key1 == *key2;
				}
			};

			class KeyHashFunctor
			{
			public:
				size_t operator()(const Handle<CallRequest>& key) const
				{
					return key->hashify();
				}
			};



			/// Needed for memory management. Cannot be private because Pattern::Cache
			/// has to call it.
			static inline void dispose(const Handle<CallRequest> &req)
			{
					//need to do nothing, the handle object knows how to remove its contents
			}

			static inline const Handle<OverloadedSet::CallResolution> &getMissedElement()
			{
				return Cache::MISSED;
			}

		}; // end of CacheTraits
public:
	/**
	 * Store an entry in the cache.
	 */
	inline void remember(const Handle<CallRequest> &req,
			Handle<OverloadedSet::CallResolution> value)
	{
		emptyCacheIfFlushed();
		actual_cache.remember(req, value);
	}

	/**
	 * Fetch an entry in the cache.
	 */
	inline const Handle<OverloadedSet::CallResolution>& recall(const Handle<CallRequest> &req)
	{
		emptyCacheIfFlushed();
		return actual_cache.recall(req);
	}

	/**
	 * Relinquish cache contents.
	 */
	static inline void flush() {
		m_globalRoundNumber++;

	}


	inline Cache() {
		m_thisCacheUpdatedRound = m_globalRoundNumber;
	}
private:

	/**
	 * When information that regards function resolution caches is updated
	 * then a new "round" starts, which means that all the cache information
	 * that was generated in a previous round is invalid.
	 * The counter helps in knowing which is the current round.
	 */
	static size_t m_globalRoundNumber;

	/**
	 * This is the round number which was valid the last time this cache
	 * was updated. If the number is not equal to the global  round number then
	 * the contents of this cache are invalid.
	 */
	size_t m_thisCacheUpdatedRound;

	/**
	 *  Since the caches are flushed lazily then
	 */
	inline void emptyCacheIfFlushed() {
		if(m_globalRoundNumber != m_thisCacheUpdatedRound) {
			m_thisCacheUpdatedRound = m_globalRoundNumber;
			actual_cache.flush();
		}
	}

	// Importing the cache pattern
	Pattern::Cache<CacheTraits> actual_cache;

	static const Handle<OverloadedSet::CallResolution> MISSED; // Missed is a null pointer
};

const Handle<OverloadedSet::CallResolution> OverloadedSet::Cache::MISSED = Handle<OverloadedSet::CallResolution>(0);
size_t OverloadedSet::Cache::m_globalRoundNumber = 0;



namespace {






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
 * Checks whether the parameters of two alternatives are absolutely identical.
 * This situation may occur if, for example, a method has a const and a
 * non-const version with the same arguments.
 */
bool identicalAlternatives(Handle<CFunction> alt1, Handle<CFunction> alt2)
{
	const RobinTypes& args1 = alt1->signature();
	const RobinTypes& args2 = alt2->signature();

	// Compare lengths
	if (args1.size() != args2.size()) return false;

	for (RobinTypes::const_iterator arg1_iter = args1.begin(),
			 arg2_iter = args2.begin();
		 arg1_iter != args1.end(); ++arg1_iter, ++arg2_iter) {
		// Compare args
		if (*arg1_iter != *arg2_iter) return false;
	}

	return true;
}



/**
 * Applies a series of conversion routes to a list
 * of actual arguments to produce a secondary list suitable for
 * calling a function directly.
 * (vector version)
 */
void convertSequence(const ActualArgumentList& original,
					 const ConversionRoutes& conversions,
					 ActualArgumentList& converted,
					 GarbageCollection& gc)
{
	assert_true(original.size() == conversions.size());
	converted.resize(original.size());
	// Apply each conversion route independently
	for (size_t i = 0; i < original.size(); ++i) {
		converted[i] = conversions[i]->apply(original[i], gc);
	}
}


/**
 * Applies a possible edge conversion which is associated with a given
 * type. The parameter 'value' initially contains the original value, and
 * is assigned the new value if a conversion actually takes place.
 */
void applyEdgeConversions(Handle<RobinType> type,
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
 * Constructor - builds a set with no
 * alternatives. Add alternatives with
 * <methodref>addAlternative()</methodref>.
 *
 */
OverloadedSet::OverloadedSet(const char *name)
	: m_refcount(new int(0)),m_name(name),m_cache(new Cache())
{

}

inline Handle<OverloadedSet> OverloadedSet::get_handler()
{
	return Handle<OverloadedSet>(this,this->m_refcount);
}

inline Handle<OverloadedSet> OverloadedSet::get_handler() const
{
	return Handle<OverloadedSet>(const_cast<OverloadedSet *>(this),this->m_refcount);
}


Handle<OverloadedSet> OverloadedSet::create_new(const char *name) {
	OverloadedSet *over = new OverloadedSet(name);
	return over->get_handler();
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
 * It finds the appropriate signature from all the available in the function,
 * for a specific call.
 * @param  req A CallRequest parameter which specifies the types of the
 * parameters, they order and which parameters were passed by name.
 * @returns A CallResolution object which specifies which of the signatures will be
 * called and which type conversions will be applied.
 *
 *
 * @param req It determines which types were passed to each parameter
 */
Handle<OverloadedSet::CallResolution> OverloadedSet::resolveCall(
		Handle<CallRequest> &req) const {

	const int nargs = req->m_nargs;


	// The return value
	Handle<CallResolution> select;


    // Locate applicable alternative from m_alternatives.
    //   Look in the cache first, and if this call is not there, perform
    //   a complete overload resolution.

	dbg::trace << "resolving call" << dbg::endl;

    // Trying to read the resolution from the cache.
    if (req->m_kwargOrder->size() == 0) {

    	Handle<CallResolution> select =
    			m_cache->recall(req);

        if(select) {
        	dbg::trace << "it was cached." << dbg::endl;
        	return select;
        } else {
        	dbg::trace << "it is not in the cache." << dbg::endl;
        }
    }

	// Now find the best sequence-route from actual_types to any of
	// the alternative prototypes
	bool ambiguity_alert = false;


	int selectedAlternativeNumber = -1;

	// When the call is ambiguos we will store a vector
	// with all the competing signatures.
	altlist otherEquivalentSignatures;


	for (altvec::const_iterator alt_iter = m_alternatives.begin();
		alt_iter != m_alternatives.end();
		++alt_iter) {

		/* Check this one out */
		int alternativeNumber =  alt_iter - m_alternatives.begin();
		dbg::trace << "// Trying signature #"
					  <<  alternativeNumber << dbg::endl;
		dbg::IndentationGuard guard(dbg::trace);

		if(! (*alt_iter)->checkProperArgumentsPassed(*req) ) {
			continue;
		}

		try {
			Handle<CallResolution> proposed(new CallResolution(req,*alt_iter));

			/* Compare the set of conversions needed to make this call
			 * with the set needed for the lightest known alternative
			 * so far */
			if(select)
			{
				switch (select->compareCost(*proposed)) {
				case CallResolution::OL_PROPOSED_BETTER:
					select = proposed;
					selectedAlternativeNumber = alternativeNumber;
					ambiguity_alert = false;
					dbg::trace << "The best signature so far." << dbg::endl;
					break;
				case CallResolution::OL_EQUIVALENT:
				case CallResolution::OL_AMBIGUOUS:
					if (!identicalAlternatives(select->m_function, *alt_iter))
					{
						if(!ambiguity_alert) {
							otherEquivalentSignatures.clear();
							ambiguity_alert = true;
						}
						otherEquivalentSignatures.push_back(*alt_iter);

					}
					break;
				case CallResolution::OL_PROPOSED_WORSE:
					break;
				}
			} else {
				// Found the first resolution (since no exception was thrown)
				select = proposed;
				selectedAlternativeNumber = alternativeNumber;
				ambiguity_alert = false;
				dbg::trace << "Signature is Ok." << dbg::endl;
			}
		}
		catch (NoApplicableConversionException& ) {
			dbg::trace << "Impossible to call this signature." << dbg::endl;
		}
	}


	// Now, if a valid alternative was found, issue the call.
	// Otherwise, issue an exception.
	if (!select) {
		dbg::trace << "Found no signatures to call.";
		throw OverloadingNoMatchException(*this,*req);
	}
	else if (ambiguity_alert) {
		dbg::trace << "Found more than one signature, cannot decide.";
		otherEquivalentSignatures.push_back(m_alternatives[selectedAlternativeNumber]);
		throw OverloadingAmbiguityException(*this,otherEquivalentSignatures);
	}
	else {
		dbg::trace << "The lightest signature was #" << selectedAlternativeNumber << dbg::endl;
		dbg::IndentationGuard guard(dbg::trace);
		for(int i =0;i<nargs;i++) {
			dbg::trace << "Conversion weight for param " << i << ": " << (*select->m_weights)[i] << dbg::endl;
		}
		if(req->m_kwargOrder->size() == 0) {
		   m_cache->remember(req,select);
		}
	}
    return select;

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
scripting_element OverloadedSet::call(const Handle<ActualArgumentList>& args,
		const KeywordArgumentMap &kwargs, scripting_element owner) const
{
	scripting_element ret;
	dbg::trace << "Calling overloaded function " << m_name << dbg::endl;
	dbg::IndentationGuard guard(dbg::trace);
	Handle<CallRequest> req(new CallRequest(*args, kwargs,this));
	Handle<CallResolution> selection = resolveCall(req);
	ret =  selection->call(args,kwargs,owner);

	return ret;
}

Handle<WeightList> OverloadedSet::weight(const Handle<ActualArgumentList>& args,
		const KeywordArgumentMap &kwargs) const
{
	Handle<WeightList> ret;
	try{
		dbg::trace << "Weighting overloaded function " << m_name << dbg::endl;
		dbg::trace.increaseIndent();
		Handle<CallRequest> req(new CallRequest(*args, kwargs,this));
		try {
			Handle<CallResolution> selection = resolveCall(req);
			ret = selection->m_weights;
		}catch (const OverloadingNoMatchException &exc) {
			ret =  Handle<WeightList>(new WeightList(args->size() + kwargs.size(),Conversion::Weight::INFINITE));
		}
		catch (const OverloadingAmbiguityException &exc) {
			ret = Handle<WeightList>(new WeightList(args->size() + kwargs.size(),Conversion::Weight::INFINITE));
		}
		catch(const NoApplicableConversionException &exc) {
			ret = Handle<WeightList>(new WeightList(args->size() + kwargs.size(),Conversion::Weight::INFINITE));
		}
		dbg::trace.decreaseIndent();
	}
	catch(...) {
		dbg::trace.decreaseIndent();
		throw;
	}
	return ret;
}

Handle<Callable> OverloadedSet::preResolveCallable() const
{

	PreResolveOverloadedSet *caching = new PreResolveOverloadedSet(this->get_handler());
	return Handle<Callable>(caching);

}


/**
 * Finds an alternative which matches the given
 * prototype exactly (without conversions).
 * <p>The return value is 'null' if no prototype matched.</p>
 */
Handle<CFunction>
OverloadedSet::seekAlternative(const RobinTypes& prototype) const
{
	for (altvec::const_iterator alt_iter = m_alternatives.begin();
		 alt_iter != m_alternatives.end(); ++alt_iter) {
		// Look for an exact match
		if ((*alt_iter)->signature() == prototype)
			return (*alt_iter);
	}

	return Handle<CFunction>();
}


bool OverloadedSet::is_empty() const
{
	return m_alternatives.empty();
}


void OverloadedSet::forceRecompute()
{
	OverloadedSet::Cache::flush();
}


void OverloadedSet::printCandidates(std::ostream &o) const
{
	printCandidates(o,m_alternatives);
}

void OverloadedSet::printCandidates(std::ostream &o, const OverloadedSet::altlist &candidates)
{
	for(altlist::const_iterator it = candidates.begin();
		it != candidates.end();
		it++)
	{
		o << **it << "\n";
	}

}

void OverloadedSet::printCandidates(std::ostream &o, const OverloadedSet::altvec &candidates)
{
	for(size_t i =0;i<candidates.size();i++)
	{
		o << *candidates[i] << "\n";
	}
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
