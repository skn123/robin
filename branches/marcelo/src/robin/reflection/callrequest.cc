/*
 * callrequest.cc
 *
 *  Created on: Oct 4, 2009
 *      Author: Marcelo Taube
 */

#include "callrequest.h"
#include "cfunction.h"
#include "../frontends/framework.h"
#include "../debug/trace.h"
#include "overloadedset.h"

namespace Robin {


/**
 * Takes a KeywordArgumentMap and produces a map of its keys in sorted order
 * This relies on the invariant that keys in an std::map are iterated on in sorted order
 */
Handle<ArgumentIndices> arrangeKWArguments(const KeywordArgumentMap& kwargs) {

        Handle<ArgumentIndices> indices(new ArgumentIndices);

        size_t index = 0;
        dbg::trace << "// @ARRANGE_KW_ARGUMENTS: asked to arrange " << kwargs.size() << " kwargs" << dbg::endl;
        for(KeywordArgumentMap::const_iterator i = kwargs.begin(); i != kwargs.end();
                                               ++i, ++index)
        {
                dbg::trace << "//\t---- arg '" << i->first << "' is at index " << index << dbg::endl;
                (*indices)[i->first] = index;
        }

        return indices;

}



CallRequest::CallRequest(
		const ActualArgumentList& args,
		const KeywordArgumentMap &kwargs,
		const OverloadedSet *calledFunc)
	: m_nargs(args.size() + kwargs.size()),
	  m_nargsPassedByName(kwargs.size()),
	  m_nargsPassedByPosition(args.size()),
	  m_calledFunc(calledFunc)
{
#ifdef IS_ARGUMENT_LIMIT
    // - we intend to use arrays, make sure the array limit is not exceeded
    if (m_nargs > OverloadedSet::ARGUMENT_ARRAY_LIMIT)
        throw ArgumentArrayLimitExceededException();
#endif

    m_types.reserve(m_nargs);
    if(m_nargsPassedByName) {
    	m_kwargOrder = arrangeKWArguments(kwargs);
    } else  {
    	m_kwargOrder = Handle<ArgumentIndices>(new ArgumentIndices());
    }

    // First taking care of the args passed by position
    for (    ActualArgumentList::const_iterator argi = args.begin();
    		argi != args.end();
         ++argi) {
        // Get type and put them in the actual_... vectors
    	m_types.push_back( FrontendsFramework::activeFrontend()->detectType_mostSpecific(*argi));
    }

    // Taking care of the args passed by name
    // Please notice that this is the same order defined by the function
    // arrangeKWArguments
    for (    KeywordArgumentMap::const_iterator argi = kwargs.begin();
    		argi != kwargs.end();
         ++argi) {
        // Get type and put them in the actual_... vectors
    	m_types.push_back( FrontendsFramework::activeFrontend()->detectType_mostSpecific(argi->second));
    }

	//Generating a vector with all the arguments names
	m_argsPassedByName.reserve(m_nargsPassedByName);
	for(ArgumentIndices::const_iterator argument = m_kwargOrder->begin();
									argument != m_kwargOrder->end();
									argument++)
	{
		m_argsPassedByName.push_back(argument->first);
	}

}

size_t CallRequest::hashify() const {
	size_t accum = 0;
	for (size_t i = 0; i < m_nargs; ++i)
	{
		// There is only one RobinType object for each type
		// so the address can be used to know if two types are the
		// same.
		accum += (size_t)&*(m_types[i]);
	}
	return accum;
}

bool CallRequest::operator<(const CallRequest &other) const {
		if (m_nargs < other.m_nargs) return true;
		if (m_nargs > other.m_nargs) return false;

		if (m_nargsPassedByPosition < other.m_nargsPassedByPosition) return true;
		if (m_nargsPassedByPosition > other.m_nargsPassedByPosition) return false;

		for (size_t i = 0; i < m_nargs; ++i) {
			if (&*m_types[i] < &*other.m_types[i]) return true;
			if (&*m_types[i] > &*other.m_types[i]) return false;
		}

		ArgumentIndices::const_iterator keyword = m_kwargOrder->begin();
		ArgumentIndices::const_iterator keyword_end = m_kwargOrder->end();
		ArgumentIndices::const_iterator keyword_other = other.m_kwargOrder->begin();

		for(;keyword != keyword_end; keyword++,keyword_other++) {
			if(keyword->first < keyword_other->first) return true;
			if(keyword->first > keyword_other->first) return false;
			if(keyword->second < keyword_other->second) return true;
			if(keyword->second > keyword_other->second) return false;
		}

		return false;
}


bool CallRequest::operator==(const CallRequest &other) const 	{

		// First comparing sizes because it is the fastest way to discard
		// Then comparing types and names of parameters.
		// Notice some fields are not checked because they have equivalent information
		// to other fields.
		return (m_nargs == other.m_nargs) &&
			   (m_nargsPassedByPosition == other.m_nargsPassedByPosition) &&
			   (m_types == other.m_types) &&
				m_argsPassedByName == other.m_argsPassedByName;
}

std::ostream &operator<<(std::ostream &o, const CallRequest &req )
{

    for(size_t i=0; i < req.m_nargsPassedByPosition; i++) {
            o << *req.m_types[i];
			if(i<req.m_types.size()-1)
			{
				 o << ", ";
			}
    }

    for(size_t i= req.m_nargsPassedByPosition ; i<req.m_types.size(); i++) {
			o << req.m_argsPassedByName[i-req.m_nargsPassedByPosition]
			  << "=" << *req.m_types[i];
			if(i<req.m_types.size()-1)
			{
				 o << ", ";
			}
	}
    return o;
}
}; //namespace Robin
