// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/typeofargument.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "robintype.h"
#include "pointer.h"
#include "class.h"
#include "enumeratedtype.h"
#include "../frontends/adapter.h"
#include <sstream>
#include "typeexistanceobservable.h"
#include "conversiontable.h"
#include "conversiontree.h"
#include "intrinsic_type_arguments.h"
#include "fundamental_conversions.h"
namespace Robin {




Type::~Type()
{
    // This destructor is here only to keep instantiation of Handle<Adapter>
    // out of the header.
}




size_t RobinType::RobinTypes_counter = 0;

ConversionProposer::~ConversionProposer()
{

};

/**
 * Builds an intrinsic argument type (category==
 * TYPE_CATEGORY_INTRINSIC or category==TYPE_CATEGORY_EXTENDED).
 * It assigns no redirection, reference or array indicators.
 */
RobinType::RobinType(TypeCategory category, TypeSpec spec, RobinType::ConstnessKind kind, bool borrowed /* = false */)
	: m_refcount(new int(1)), m_constTypeAdditionAnnouncer(new TypeExistanceObservable()),
	m_borrowed(borrowed), m_id(++RobinTypes_counter)
{
	m_basetype.category = category;
    m_basetype.spec = spec;
    m_basetype.name = 0;
	m_constType =  kind;
}


/**
 * Builds an intrinsic argument type (category==
 * TYPE_CATEGORY_INTRINSIC or category==TYPE_CATEGORY_EXTENDED).
 * It assigns no redirection, reference or array indicators.
 */
RobinType::RobinType(TypeCategory category, TypeSpec spec, const char*name, RobinType::ConstnessKind kind,bool borrowed /* = false */)
	: m_refcount(new int(1)), m_constTypeAdditionAnnouncer(new TypeExistanceObservable()),
	m_borrowed(borrowed),
	 m_id(++RobinTypes_counter)
{
    m_basetype.category = category;
    m_basetype.spec = spec;
    m_basetype.name = name;
	m_constType =  kind;
}

/**
 * Builds an argument of a class type. Assigns the
 * value TYPE_CATEGORY_USERDEFINED to category and TYPE_USERDEFINED_OBJECT
 * to spec.
 */
RobinType::RobinType(Handle<Class> classtype, RobinType::ConstnessKind kind, bool borrowed /* = false */)
	: m_refcount(new int(1)), m_constTypeAdditionAnnouncer(new TypeExistanceObservable()),
	m_borrowed(borrowed),
	 m_id(++RobinTypes_counter)
{
    m_basetype.category = TYPE_CATEGORY_USERDEFINED;
    m_basetype.spec = TYPE_USERDEFINED_OBJECT;
    m_basetype.objclass = classtype;
    m_basetype.name = 0;
    m_constType = kind;
}

/**
 * Builds an argument of an enumerated type. assigns the
 * value TYPE_CATEGORY_USERDEFINED to category and TYPE_USERDEFINED_ENUM
 * to spec.
 */
RobinType::RobinType(Handle<EnumeratedType> enumtype, RobinType::ConstnessKind kind, bool borrowed /* = false */)
	: m_refcount(new int(1)), m_constTypeAdditionAnnouncer(new TypeExistanceObservable()),
	m_borrowed(borrowed),
	m_id(++RobinTypes_counter)
{
    m_basetype.category = TYPE_CATEGORY_USERDEFINED;
    m_basetype.spec = TYPE_USERDEFINED_ENUM;
    m_basetype.objenum = enumtype;
    m_basetype.name = 0;
	m_constType =  kind;
}



Handle<RobinType> RobinType::create_new(TypeCategory category, TypeSpec spec, const char * name, RobinType::ConstnessKind kind, bool borrowed)
{
	RobinType *self = new RobinType(category, spec, name, kind, borrowed);
	Handle<RobinType> ret = self->get_handler();

	// m_refcount was first set to 1 to make sure the class
	// is not deleted till we return the handler, but now
	// that we have a handler we are safe
	*self->m_refcount -= 1;
	return ret;
}

Handle<RobinType> RobinType::create_new(TypeCategory category, TypeSpec spec, RobinType::ConstnessKind kind,bool borrowed)
{
	RobinType *self = new RobinType(category, spec, kind,borrowed);
	Handle<RobinType> ret = self->get_handler();

	// m_refcount was first set to 1 to make sure the class
	// is not deleted till we return the handler, but now
	// that we have a handler we are safe
	*self->m_refcount -= 1;
	return ret;
}


Handle<RobinType> RobinType::create_new(Handle<Class> cls, RobinType::ConstnessKind kind, bool borrowed)
{
	RobinType *self = new RobinType(cls, kind, borrowed);
	Handle<RobinType> ret = self->get_handler();

	// m_refcount was first set to 1 to make sure the class
	// is not deleted till we return the handler, but now
	// that we have a handler we are safe
	*self->m_refcount -= 1;
	return ret;
}

Handle<RobinType> RobinType::create_new(Handle<EnumeratedType> enu,RobinType::ConstnessKind kind, bool borrowed)
{
	RobinType *self = new RobinType(enu, kind, borrowed);
	Handle<RobinType> ret = self->get_handler();

	// m_refcount was first set to 1 to make sure the class
	// is not deleted till we return the handler, but now
	// that we have a handler we are safe
	*self->m_refcount -= 1;
	return ret;
}




/**
 * Releases resources associated with the argument type,
 * such as the <classref>Adapter</classref>.
 */
RobinType::~RobinType()
{
    // This destructor is here only to keep instantiation of Handle<Adapter>
    // out of the header.
}



/**
 * Returns a <classref>Type</classref> structures which
 * helps to identify the type this RobinType object denotes.
 * This information is useful for frontend writers.
 */
Type RobinType::basetype() const
{
    return m_basetype;
}

/**
 * Whatever this is a constant reference type.
 * It is used for example to know if values have to be updated back
 * after calling a function which receives this.
 * Other concepts which are pretty close.
 * 		->  An rvalue in C++
 *      ->  A reference to a python immutable behaves like a constant
 *         reference when using it as a parameter because nothing can
 *         be changed outside the function.
 */
RobinType::ConstnessKind RobinType::isConstant() const
{
	return m_constType;
}


/**
 * Checks whether this type is a reference-to-object type.
 *
 * @return true if this is a reference type
 */
bool RobinType::isReference() const
{
	return  (m_basetype.category == TYPE_CATEGORY_USERDEFINED &&
	         m_basetype.spec == TYPE_USERDEFINED_OBJECT &&
	         this == &*(m_basetype.objclass->getConstType()));
}

/**
 * Checks whether this type is borrowed
 *
 * @return true if this is a borrowed type
 */
bool RobinType::isBorrowed() const
{
	return m_borrowed;
}


bool RobinType::isHyperGeneric() const
{
	return false;
}

/**
 * Returns a handle to a RobinType which represents a pointer to the
 * specified type. Two consecutive calls to pointer() will always return the
 * same object.
 */
Handle<RobinType> RobinType::pointer() const
{
	if (!m_cache_pointer) {
		m_cache_pointer = PointerType::create_new(*this);
	}
	return m_cache_pointer;
}


/**
 * Associates an <classref>Adapter</classref> object
 * with the argument. This Adapter should know how to translate the
 * <b>base type</b> alone - any redirection/dereferencing is done
 * by the RobinType::get and RobinType::put methods.
 */
void RobinType::assignAdapter(Handle<Adapter> adapter)
{
    m_adapter = adapter;
}

/**
 * @par DESCIPRTION
 * Translates a return value using the associated
 * Adapter. The return value is a scripting_element in the terms
 * of the active frontend.
 */
scripting_element RobinType::get(basic_block return_value)
{
    if (m_adapter)
		return m_adapter->get(return_value);
    else
		throw UnsupportedInterfaceException();
}

/**
 * Translate an argument given in the terms of the
 * active scripting environment frontend to C standards and pushes
 * it on the <classref>ArgumentsBuffer</classref>.
 */
void RobinType::put(ArgumentsBuffer& argsbuf, scripting_element value)
{
    if (m_adapter)
		m_adapter->put(argsbuf, value);
    else
		throw UnsupportedInterfaceException();
}



const char *SPEC_NAMES[] =
{
	    "c_int",
	    "uint",
	    "c_long",
	    "longlong",
	    "ulong",
	    "ulonglong",
	    "char",
	    "schar",
	    "uchar",
	    "short",
	    "ushort",
	    "longlong",
	    "ulonglong",
	    "c_float",
	    "double",
	    "c_bool",
	    "c_void",
	    "c_string",
	    "pascal_string",
	    "scripting_element",
	    "object",
	    "enum",
	    "unknown interceptor",
	    "pointer"
};

std::string RobinType::getTypeName() const
{
	//make sure the table of spec names include all the types
	assert_true(sizeof(SPEC_NAMES) == TYPE_SPEC_SIZE*sizeof(char *));
	std::stringstream out;
	if(isConstant()) {
		out << "const ";
	}
	if(m_basetype.category == TYPE_CATEGORY_USERDEFINED
			&& m_basetype.spec == TYPE_USERDEFINED_OBJECT
			&& m_basetype.objclass)
	{
		out << m_basetype.objclass->name();
		if (isBorrowed()) {
			out << " <";
		}
		if (m_basetype.objclass->getPtrType()->getID() == this->getID())
		{
			out << " *";
		}
	} else if(m_basetype.name) {
		out << m_basetype.name;
	} else {
		out << SPEC_NAMES[m_basetype.spec];
	}
	return out.str();
}

/**
 * Adds trivial conversions to the given adjacency list.
 */
void RobinType::addTrivialConversions(
		ConversionTable::AdjacencyList &list) const
{
	ConversionTable::Adjacency adj;
	adj.targetNode = ArgumentScriptingElementNewRef;
	adj.edge = Handle<Conversion>(new TrivialConversion);
	adj.edge->setSourceType(this->get_handler());
	adj.edge->setTargetType(adj.targetNode);
	list.push_back(adj);
}


void RobinType::setConversionProposer(const Handle<ConversionProposer> &proposer)
{
	assert_true(!m_proposer);
	m_proposer = proposer;
}

void RobinType::proposeConversionContinuations(
		 const Conversion::Weight &reachedWeight,
		 AssocHeap<Conversion::Weight,const RobinType*> & bestWeightHeap,
		 bool constConversions,
		 TypeToWeightMap & distanceMap,
		 ConversionTree & previousStepMap) const
{
	if(m_proposer) {
		m_proposer->proposeConversionContinuations(
				reachedWeight,
				bestWeightHeap,
				constConversions,
				distanceMap,
				previousStepMap);
	}

	typedef ConversionTable::AdjacencyList AdjacencyList;


	/* for each vertex v in Adj[u] */
	AdjacencyList adj_u = ConversionTableSingleton::getInstance()->getAdjacentTo(*this);
	addTrivialConversions(adj_u);
	for (AdjacencyList::const_iterator v_i = adj_u.begin();
		 v_i != adj_u.end(); ++v_i)
	{
		const RobinType *v = &*(v_i->targetNode);
		if(constConversions && !v->isConstant())
		{
			continue;
		}

		Conversion::Weight w =  v_i->edge->weight();
		// DEBUG CODE
		// {
#			if ROBIN_DEEP_DEBUG_CONVERSIONS
			Handle<RobinType> dbg_source = v_i->edge->sourceType();
			dbg::trace << "Considering edge from: ";
				if(dbg_source) {
					dbg::trace <<*dbg_source;
				} else {
					dbg::trace << "unknown";
				}

				dbg::trace << " to " << *v_i->edge->targetType() <<
				" with price" << w << dbg::endl;
#			endif
		// } // END OF DEBUG CODE


		Conversion::Weight sum = reachedWeight + w;
		if (distanceMap.updateWeightIfBetter(v,sum)) {
			const Handle<RobinType> &target = v_i->targetNode;
			previousStepMap[target] = v_i->edge /* (u, v) */;
			bestWeightHeap.updateWeight(v,sum);
		}
	}

}

std::ostream &operator<<(std::ostream &out, const RobinType &type) {
	out << type.getTypeName();
	return out;
}

RobinTypeAddedObserver::~RobinTypeAddedObserver() {

}

} // end of namespace Robin
