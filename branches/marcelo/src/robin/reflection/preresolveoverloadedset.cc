/*
 * PreResolveOverloadedSet.cc
 *
 *  Created on: Oct 7, 2009
 *      Author: Marcelo Taube
 */

#include "preresolveoverloadedset.h"
#include "callresolution.h"
#include "overloadedset.h"
#include "conversiontable.h"
#include "../debug/trace.h"
namespace Robin {

PreResolveOverloadedSet::PreResolveOverloadedSet(const Handle<OverloadedSet> &overloadedSet)
: m_overloadedSet(overloadedSet)
{

}

PreResolveOverloadedSet::PreResolveOverloadedSet(const PreResolveOverloadedSet &other)
: m_overloadedSet(other.m_overloadedSet), m_resolutions(other.m_resolutions)
{

}
scripting_element
PreResolveOverloadedSet::call(const Handle<ActualArgumentList>& args,
		     const KeywordArgumentMap &kwargs,
		     scripting_element owner) const
{

	dbg::trace << "Calling pre-resolved overloaded function " << m_overloadedSet->m_name << dbg::endl;
	dbg::IndentationGuard guard(dbg::trace);

	ResolutionsList::const_iterator resolutionIt = m_resolutions.begin();
	ResolutionsList::const_iterator resolutionEnd = m_resolutions.end();


	Handle<CallRequest> req(new CallRequest(*args,kwargs,&*m_overloadedSet));
	for(; resolutionIt != resolutionEnd; resolutionIt++)
	{

		if(*(*resolutionIt)->m_request == (*req)) {
			dbg::trace << "Trying to call pre resolved function " << m_overloadedSet->m_name <<  "with one of its resolutions"<< dbg::endl;
			dbg::IndentationGuard internalGuard(dbg::trace);
			scripting_element ret=  (*resolutionIt)->call(args,kwargs,owner);
			return ret;
		}
	}

	Handle<OverloadedSet::CallResolution> resolve = m_overloadedSet->resolveCall(req);
	m_resolutions.push_back(resolve);
	return resolve->call(args,kwargs,owner);
}


Handle<WeightList> PreResolveOverloadedSet::weight(const Handle<ActualArgumentList>& args,
	     const KeywordArgumentMap &kwargs) const
{
	ResolutionsList::const_iterator resolutionIt = m_resolutions.begin();
	ResolutionsList::const_iterator resolutionEnd = m_resolutions.end();

	// TODO (marcelo taube) :  debugging dangerous code
	if(resolutionIt!=resolutionEnd)
	{
		try {
			dbg::trace << "Weighthing pre resolved function " << m_overloadedSet->m_name << dbg::endl;
			dbg::trace.increaseIndent();
			Handle<WeightList> ret=  (*resolutionIt)->m_weights;
			dbg::trace.decreaseIndent();
			return ret;
		} catch(...) {
			dbg::trace.decreaseIndent();
			throw;
		}

	}

	Handle<CallRequest> req(new CallRequest(*args,kwargs,&*m_overloadedSet));
	try {
		Handle<OverloadedSet::CallResolution> resolve = m_overloadedSet->resolveCall(req);
		m_resolutions.push_back(resolve);
		return resolve->m_weights;
	}catch (const OverloadingNoMatchException &exc) {
		return  Handle<WeightList>(new WeightList(args->size() + kwargs.size(),Conversion::Weight::INFINITE));
	}
	catch (const OverloadingAmbiguityException &exc) {
		return Handle<WeightList>(new WeightList(args->size() + kwargs.size(),Conversion::Weight::INFINITE));
	}
	catch(const NoApplicableConversionException &exc) {
		return Handle<WeightList>(new WeightList(args->size() + kwargs.size(),Conversion::Weight::INFINITE));
	}
}


Handle<Callable> PreResolveOverloadedSet::preResolveCallable() const
{
	return Handle<Callable>(new PreResolveOverloadedSet(*this));
}

}; // end of namespace Robin
