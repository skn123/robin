/*
 * conversiontree.h
 *
 *  Created on: Feb 22, 2010
 *      Author: Marcelo Taube
 */

#ifndef ROBIN_REFLECTION_CONVERSIONTREE_H_
#define ROBIN_REFLECTION_CONVERSIONTREE_H_

#include<map>
#include<pattern/handle.h>

namespace Robin {

class ConversionRoute;
class RobinType;
class Conversion;

/**
 * It is a directed tree, where nodes are RobinTypes and edges are
 * Conversions between those.
 */
class ConversionTree : public std::map<const Handle<RobinType> ,Handle<Conversion> >
{
public:

	/**
	 * It constructs a Conversion tree.
	 * It is initialized to have no edges and one node which is
	 * the source of the tree.
	 */
	ConversionTree(const RobinType &source);

	/**
	 * Returns true if the connecting Conversion to 'dest' is
	 * of a certain kind.
	 *
	 * By 'kind' we mean a C++ class (subclass of Converson).
	 *
	 * @template Kind is a kind inherting from Conversion.
	 */
	template <typename Kind>
	inline bool previousConversionOfKind(const Handle<RobinType> &dest)
	{
		 std::map<Handle<RobinType> ,Handle<Conversion> >::iterator
			 previous_it = find(dest);
		 return previous_it != end() &&
		    dynamic_cast<Kind*>(previous_it->second.pointer());

	}

	/**
	 *  Returns a conversion route using the previous steps map
	 *  and following the map back till there is no more previous steps.
	 *
	 *  @throws NoApplicableConversionException if 'dest' is not part of
	 *  the tree
	 */
	Handle<ConversionRoute> generateRouteTo(const Handle<RobinType> &dest);

private:
	const RobinType &m_source;
};

}; //end of namespace Robin
#endif /* ROBIN_REFLECTION_CONVERSIONTREE_H_ */
