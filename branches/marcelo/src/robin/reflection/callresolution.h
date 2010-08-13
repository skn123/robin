/*
 * callresolution.h
 *
 *  Created on: Oct 8, 2009
 *      Author: Marcelo Taube
 */

#ifndef CALLRESOLUTION_H_
#define CALLRESOLUTION_H_

#include "overloadedset.h"
#include "callrequest.h"
#include "conversion.h"


namespace Robin {



class OverloadedSet::CallResolution
{
public:
		 /**
		  *  Construct a call resolution by choosing a specific function
		  *  and the parameter types.
		  *  The constructor itself calculates the conversions
		  *
		  *  Finds the best conversion paths for eachparameter type.
		  *
		  */
	     CallResolution( Handle<CallRequest> &req,
							const Handle<CFunction> &calledFunc);


	     /**
	      *   Uses this CallResolution to do a function call.
	      *   Of course the parameters have to be of the type
	      *   expected by this call resolution
	      */
	     scripting_element call(const Handle<ActualArgumentList>& args,
	     		const KeywordArgumentMap &kwargs, scripting_element owner) const;


	     enum overloading_relationship {
	     	OL_PROPOSED_BETTER,
	     	OL_PROPOSED_WORSE,
	     	OL_EQUIVALENT,
	     	OL_AMBIGUOUS
	     };

	     overloading_relationship compareCost(const CallResolution & proposedResolution);

		/**
		 * A particular function signature to call
		 */
		Handle<CFunction> m_function;

		/**
		 * Conversion routes to perform when doing the call
		 */
		ConversionRoutes m_routes;


		/**
		 *  A reordered signature is the signature of the function
		 *  but the types which were passed by name are reordered
		 *  lexicographically (so they
		 *  match how the actual parameters are ordered in the CallRequest object.)
		 *
		 *  It is important to notice that in the CallResolution the function parameters
		 *  changes to fit the order of the actual parameters, and not the reverse.
		 *  Only later the call itself has
		 *  to be reordered back to call the C code.
		 */
		Handle<RobinTypes> m_reordered_signature;

		/**
		 * It stores the name of the parameters in the same order
		 * that m_reordered_signature
		 */
		Handle<std::vector<std::string> > m_reordered_signature_names;


		/**
		 * The request which is resolved.
		 * Notice that the order of the parameters of the function call might be
		 * affected by the Request object. Because of that, m_routes cannot be
		 * interpreted without having the call request.
		 */
		Handle<CallRequest> m_request;


		/**
		 * The weightlist fo this resolution
		 */
		Handle<WeightList> m_weights;

private:

		/**
		 *  This is a cache of the value m_routes.areAllEmptyConversions() since
		 *  the conversions are not changed after the CallResolution is constructed.
		 */
		bool m_allConversionsAreEmpty;
};

}; // end of namespace Robin
#endif /* CALLRESOLUTION_H_ */
