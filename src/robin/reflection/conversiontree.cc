/*
 * conversiontree.cc
 *
 *  Created on: Feb 22, 2010
 *      Author: Marcelo Taube
 */

#include "conversiontree.h"
#include "robintype.h"
#include "conversion.h"

namespace Robin {


ConversionTree::ConversionTree(const RobinType& source)
	: m_source(source)
{

}

Handle<ConversionRoute> ConversionTree::generateRouteTo(const Handle<Robin::RobinType> &dest)
{
	Handle<ConversionRoute> hroute(new ConversionRoute);
	ConversionRoute& route = *hroute;

	Handle<RobinType> tail = dest;
	while(tail.pointer() != &m_source) {
		iterator edge_it = find(tail);
		if(edge_it == end())
		{
		    dbg::trace << "// @TYPE-DISTANCE: infinite" << dbg::endl;
			throw NoApplicableConversionException(&m_source, dest.pointer());
		}

		Handle<Conversion> &edge = edge_it->second;
		if(!edge->isZeroWorkConversion())
		{
			route.insert(route.begin(), edge);
		} else {
			route.addExtraWeight(edge->weight());
		}

		tail = edge->sourceType();
	}

	return hroute;
}

}//end of namespace Robin
