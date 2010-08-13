/*
 * listrobintype.cc
 *
 *  Created on: Jan 28, 2010
 *      Author: Marcelo Taube
 */

#include <Python.h>
#include "../port.h"
#include "listrobintype.h"
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/conversiontree.h>
#include <robin/reflection/fundamental_conversions.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/const.h>
#include <robin/debug/assert.h>
#include <robin/frontends/python/pyhandle.h>
#include <robin/reflection/memorymanager.h>
using namespace std;

namespace Robin {
namespace Python {


/**
 * A RobinType which represents an empty python list.
 */
class EmptyPythonListType : public RobinType
{
public:

	/**
	 * Destructor
	 */
	virtual ~EmptyPythonListType()
	{

	}

	/**
	 *  The name of the type, can be used for debug prints.
	 */
	 virtual string getTypeName() const
	 {
		 return "empty PyList";
	 }

	 /*
	  * @name Factory functions
	  */
	 //@{
		 static Handle<EmptyPythonListType> create_new()
		{
			EmptyPythonListType *self = new EmptyPythonListType();
			Handle<EmptyPythonListType> h_self = ::static_hcast<EmptyPythonListType>(self->get_handler());

			// m_refcount was first set to 1 to make sure the class
			// is not deleted till we return the handler, but now
			// that we have a handler we are safe
			*self->m_refcount -= 1;
			return h_self;
		}
	 //@}


	 bool isHyperGeneric() const
	 {
		 return true;
	 }
protected:
	/**
	 * @name Constructors
	 */
	//@{

		 EmptyPythonListType()
			 : RobinType(TYPE_CATEGORY_EXTENDED, TYPE_EXTENDED_VOID, "PyList", RobinType::regularKind)
		 {

		 }

	//@}

};


/**
 * A RobinType which represents any python list which its
 * elements might not be specific of a certain type.
 */
class GeneralPythonListType : public RobinType
{
public:

	/**
	 * Destructor
	 */
	virtual ~GeneralPythonListType()
	{

	}

	/**
	 *  The name of the type, can be used for debug prints.
	 */
	 virtual string getTypeName() const
	 {
		 return "general PyList";
	 }

	 /*
	  * @name Factory functions
	  */
	 //@{
		 static Handle<GeneralPythonListType> create_new()
		{
			GeneralPythonListType *self = new GeneralPythonListType();
			Handle<GeneralPythonListType> h_self = ::static_hcast<GeneralPythonListType>(self->get_handler());

			// m_refcount was first set to 1 to make sure the class
			// is not deleted till we return the handler, but now
			// that we have a handler we are safe
			*self->m_refcount -= 1;
			return h_self;
		}
	 //@}
protected:
	/**
	 * @name Constructors
	 */
	//@{

		 GeneralPythonListType()
			 : RobinType(TYPE_CATEGORY_EXTENDED, TYPE_EXTENDED_VOID, "PyList", RobinType::regularKind)
		 {

				 Handle<RobinType> selfHandle = this->get_handler();

				/*
				 * Generating a conversion from an empty list type to this type
				 */
				Handle<Conversion> hfromemptylist(new TrivialConversion);
				hfromemptylist    ->setSourceType(ListRobinType::getEmptyListRobinType());
				hfromemptylist    ->setTargetType(selfHandle);
				ConversionTableSingleton::getInstance()->registerConversion(hfromemptylist);

				/*
				 * Generating a conversion from an empty list type to this type
				 */
				{
					Handle<Conversion> hfromconstemptylist(new TrivialConversion);
					hfromconstemptylist    ->setSourceType(ConstType::searchOrCreate(*ListRobinType::getEmptyListRobinType()));
					hfromconstemptylist    ->setTargetType(ConstType::searchOrCreate(*selfHandle));
					ConversionTableSingleton::getInstance()->registerConversion(hfromconstemptylist);
				}

				/*
				 * Generating the conversion to scripting_element, for
				 * functions which knows how to handle python objects directly
				 */
				{
					Handle<Conversion> hlist2element(new TrivialConversion);
					hlist2element    ->setSourceType(selfHandle);
					hlist2element    ->setTargetType(ArgumentScriptingElementNewRef);
					ConversionTableSingleton::getInstance()->registerConversion(hlist2element);
				}


		 }

	//@}

};

/**
 * It is the traits for a Map with key a pair of a robin type and
 * a RobinType::ConstnessKind.
 * The value is not specified, so to properly use this traits you
 * need to inherit from it and specify the value.
 */
struct TraitsForTypeKeys {
	typedef Handle<RobinType> Key;
	class KeyCompareFunctor
	{
	public:
		bool operator()(const Key& key1,
						const Key& key2) const
		{

			return key1->getID() < key2->getID();
		}
	};

	class KeyIdentityFunctor
	{
	public:
		bool operator()(const Key& key1,
						const Key& key2) const
		{
			return key1->getID() == key2->getID();
		}
	};

	class KeyHashFunctor
	{
	public:
		size_t operator()(const Key& key) const
		{
			return  key->getID();
		}
	};
};

/**
 * Traits structure for Pattern::Map
 */
struct TraitsForType2Type  : public TraitsForTypeKeys
{
			typedef Handle<RobinType> Value;
};

typedef Pattern::Map<TraitsForType2Type> Type2TypeMap;





class ListRobinType::Implementation {
private:
	Implementation()
	{

	}
public:
	static Implementation *get()
	{
		static Handle<Implementation> imp(new Implementation());
		return &*imp;
	}


	Type2TypeMap mapTypeToItsList;

};

/**
 * It is a kind of conversion between lists which internally only
 * does an element by element conversion, and the resulting list is
 * a constant reference, which means that modifications in the new list
 * will not affect the original one.
 * The internal conversion is specified by another Conversion object.
 * The type of the objects in the list must be from a const kind, that is
 * because the values on the original object will not be updated.
 */
class ConstListComposedConversion : public Conversion {
public:
	/**
	 * @name Constructors
	 */

	//@{
	ConstListComposedConversion(Handle<ConversionRoute> elementConversion)
		: m_elementConversion(elementConversion)
	{
		setWeight(m_elementConversion->totalWeight());
		//elements of the list are consts
		assert_true(elementConversion->hasOnlyConstantConversions());	}
	//@}

	/**
	 * @name Access
	 */
	//@{
	virtual bool isZeroWorkConversion() const
	{
		return m_elementConversion->isZeroWorkConversionRoute();	}
	//@}

	/**
	 * @name Activity
	 */

	//@{
	virtual scripting_element apply(scripting_element value) const
	{
		if(isZeroWorkConversion()) {
			Py_XINCREF((PyObject *)value);
			return value;
		}

		PyObject *object = (PyObject *)value;
		assert_true(PyObject_TypeCheck(object,&PyList_Type));
		Py_ssize_t len = PyList_Size(object);
		PyObject *newList = PyList_New(len);
		Py_ssize_t i;
		GarbageCollection garbageCollection;
		try {
			for(i = 0;i<len;i++)
			{
				PyReferenceBorrow<PyObject> elem_before (
						PyList_GET_ITEM(object,i));
				PyReferenceCreate<PyObject> elem_after(
						(PyObject*)m_elementConversion->apply(elem_before.pointer(),garbageCollection));

				// Notice that this call DOES NOT release the previous
				// reference at position i in the list.
				PyList_SET_ITEM(newList,i,elem_after.release());
			}

			return newList;
		}
		catch(exception &e) {
			// did not finish creating the list, need to fill
			// it with 'None' before reducing the reference count
			for(;i<len;i++) {
				Py_XINCREF(Py_None);
				PyList_SET_ITEM(newList,i,Py_None);
			}
			Py_XDECREF(newList);
			throw;
		};
	}

	//@}
protected:
	Handle<ConversionRoute> m_elementConversion;
};


/**
 * It is a kind of conversion between lists which
 * does an element by element conversion in-place, and the resulting list is
 * a non-const reference which actually points to the same list as the original.
 * The internal conversion is specified by another Conversion object.
 * The type of the objects in the list must be from a const kind, that is
 * because the values on the original object will not be updated, they will
 * only be replaced.
 */
class NonConstListComposedConversion : public Conversion {
public:
	/**
	 * @name Constructors
	 */

	//@{
	NonConstListComposedConversion(Handle<ConversionRoute> elementConversion)
		: m_elementConversion(elementConversion)
	{
		setWeight(m_elementConversion->totalWeight());
		//elements of the list are consts
		assert_true(elementConversion->hasOnlyConstantConversions());
	}
	//@}

	/**
	 * @name Access
	 */
	//@{
	virtual bool isZeroWorkConversion() const
	{
		return m_elementConversion->isZeroWorkConversionRoute();
	}
	//@}

	/**
	 * @name Activity
	 */

	//@{
	virtual scripting_element apply(scripting_element value) const
	{
		if(isZeroWorkConversion()) {
			Py_XINCREF((PyObject *)value);
			return value;
		}

		PyObject *object = (PyObject *)value;
		assert_true(PyObject_TypeCheck(object,&PyList_Type));
		Py_ssize_t len = PyList_Size(object);
		Py_ssize_t i;

		GarbageCollection garbageCollection;
		for(i = 0;i<len;i++)
		{
			PyReferenceBorrow<PyObject> elem_before (
					PyList_GET_ITEM(object,i));
			PyReferenceCreate<PyObject> elem_after(
					(PyObject*)m_elementConversion->apply(elem_before.pointer(),garbageCollection));


			// Notice that this call DOES release the previous reference at
			// position i in the list.
			PyList_SetItem(object,i,elem_after.release());
		}
		Py_XINCREF(object);
		return object;
	}

	//@}
protected:
	Handle<ConversionRoute> m_elementConversion;
};

Handle<RobinType> ListRobinType::getGeneralListRobinType()
{
	static Handle<RobinType> generalPythonList
		= ::static_hcast<RobinType>(GeneralPythonListType::create_new());
	return generalPythonList;

}

Handle<RobinType> ListRobinType::getEmptyListRobinType()
{
	static Handle<RobinType> emptyPythonList
		= ::static_hcast<RobinType>(EmptyPythonListType::create_new());
	return emptyPythonList;
}


ListRobinType::ListRobinType(Handle<RobinType> elements_type)
	: RobinType(TYPE_CATEGORY_EXTENDED, TYPE_EXTENDED_VOID, "PyList", regularKind) ,m_elements_type(elements_type)
{
}

/**
 * Generates conversions on the fly for the conversion
 * search algorithm.
 *
 * The conversions will convert from a specific listType
 * or from a constListType.
 */
class ListConversionProposer : public ConversionProposer
{
private:

	ListRobinType *m_listType;
	ConstType *m_constListType;
	bool m_isConstListProposer;
public:
	/**
	 * It constructs a ListConversionProposer for a specific type of
	 * list or constant type of list.
	 * Notice that the two types have to be passed but the conversion
	 * will be from both types.
	 *
	 * @param listType
	 * @param constListType
	 * @param constListConverter whatever this proposer will be used
	 * 							to generate conversions from the listType
	 * 							or from the constListType
	 */
	ListConversionProposer(ListRobinType* listType,ConstType* constListType,bool constListConverter);

	virtual void proposeConversionContinuations(
			 const Conversion::Weight &reachedWeight,
			 AssocHeap<Conversion::Weight,const RobinType*> & bestWeightHeap,
			 bool constConversions,
			 TypeToWeightMap & distanceMap,
			 ConversionTree & previousStepMap) const;

};

ListConversionProposer::ListConversionProposer(ListRobinType *listType,
		ConstType *constListType, bool isConstListProposer)

	:	m_listType(listType), m_constListType(constListType), m_isConstListProposer(isConstListProposer)
{

}

void ListConversionProposer::proposeConversionContinuations(
		 const Conversion::Weight &reachedWeight,
		 AssocHeap<Conversion::Weight,const RobinType*> & bestWeightHeap,
		 bool constConversions,
		 TypeToWeightMap & distanceMap,
		 ConversionTree & previousStepMap) const
{

	if(m_isConstListProposer) {
		// cannot do a nonconst conversion from a const list
		if(!constConversions){
			return;
		}
	} else {
		if(constConversions) {
			// this proposer wont add any const conversion from a non
			// const list
			// (of course the regular conversions in the conversion table
			// will still happen).
			return;
		}
	}

	 Handle<RobinType> sourceType;
	 if(m_isConstListProposer)
	 {
		 sourceType = m_constListType->get_handler();
	 } else {
		 sourceType = m_listType->get_handler();
	 }


	 /*
	  * If the previous step to us was not a list type then
	  * there is no reason to concatenate.
	  */
	 if(   previousStepMap.previousConversionOfKind<ConstListComposedConversion>(sourceType)
		|| previousStepMap.previousConversionOfKind<NonConstListComposedConversion>(sourceType))
	 {
		return;
	 }

	/*
	 * Now doing an internal type conversion search in the elements
	 */
	 Handle<ConversionTree>   previousStepMapElement;


	 /*
	  * Searching const conversions for elements
	  */
	 {
#		 if ROBIN_DEEP_DEBUG_CONVERSIONS
			 dbg::trace << "Considering internal element conversions to generate edges from "  << *sourceType << dbg::endl;
			 dbg::IndentationGuard guard(dbg::trace);
#		 endif
		 ConversionTable *convTable = ConversionTableSingleton::getInstance();
		 previousStepMapElement = convTable->generateConversionTree(*m_listType->m_elements_type,0, true);
	 }


	 /*
	  * Go through all the list types that exist
	  */
	 static ListRobinType::Implementation *imp = ListRobinType::Implementation::get();
	 Type2TypeMap::const_iterator it = imp->mapTypeToItsList.begin();
	 Type2TypeMap::const_iterator end = imp->mapTypeToItsList.end();
	 for(;it!=end;it++) {
		 Handle<RobinType> elementTargetType = it->first;
		 Handle<RobinType> listTargetType;
		 if(m_isConstListProposer)
		 {
				listTargetType = ConstType::searchOrCreate(*it->second);
		 } else {
				listTargetType = it->second;
		 }

		 /*
		  * Check if the element type can be reached, if it can
		  * then add the list conversion too.
		  */
		 ConversionTree::iterator distance_it = previousStepMapElement->find(elementTargetType);
		 if(distance_it != previousStepMapElement->end())
		 {
			Conversion::Weight sum = reachedWeight + distance_it->second->weight();
			Handle<RobinType> targetType;

			if (distanceMap.updateWeightIfBetter(listTargetType.pointer(),sum))
			{
				Handle<Conversion> newEdge;
				Handle<ConversionRoute> elementRoute = previousStepMapElement->generateRouteTo(elementTargetType);
				if(m_isConstListProposer) {
					newEdge = Handle<Conversion>(
							new ConstListComposedConversion(elementRoute));
				} else {
					newEdge = Handle<Conversion>(
							new NonConstListComposedConversion(elementRoute));
				};
				newEdge->setWeight(sum);
				newEdge->setSourceType(sourceType);
				newEdge->setTargetType(listTargetType);
				previousStepMap[listTargetType] = newEdge /* (u, v) */;
#				if ROBIN_DEEP_DEBUG_CONVERSIONS
					dbg::trace << "Use dynamic edge from: ";
						dbg::trace <<*sourceType;
						dbg::trace << " to " << *listTargetType <<
						" with price" << sum << dbg::endl;
#				endif
				bestWeightHeap.updateWeight(listTargetType.pointer(),sum);
			}
		 }
	 }

}



/**
 *  Method which takes care of registering standard conversions from/to
 *  the ListRobinType.
 *
 *  Warning this method might produce a recursive creation of other
 *  ListRobinTypes. The recursion could end up in an infinite loop if
 *  other Lists try to register conversions to us.
 *  To avoid this the method initializeConversions should only be called
 *  after the ListRobinType has been cached in
 *  ListRobinType::Implementation::mapTypeAndConstnessToItsList
 */
void ListRobinType::initializeConversions() {
	Handle<ListRobinType> selfHandle = this->get_handler();
	Handle<ConstType> selfConstHandle = ConstType::searchOrCreate(*selfHandle);

	Handle<ListConversionProposer> proposer(
			new ListConversionProposer(selfHandle.pointer(),selfConstHandle.pointer(),false));



	this->setConversionProposer(proposer);
	/*
	 * Generating the conversion to scripting_element, for
	 * function which know how to handle python objects directly
	 */
	{
		Handle<Conversion> htogenerallist(new TrivialConversion);
		htogenerallist    ->setSourceType(selfHandle);
		htogenerallist    ->setTargetType(getGeneralListRobinType());
		ConversionTableSingleton::getInstance()->registerConversion(htogenerallist);
	}

	Handle<ListConversionProposer> constProposer(
			new ListConversionProposer(selfHandle.pointer(),selfConstHandle.pointer(),true));
	selfConstHandle->setConversionProposer(constProposer);


	{
		Handle<Conversion> htogenerallistconst(new TrivialConversion);
		htogenerallistconst    ->setSourceType(selfConstHandle);
		htogenerallistconst    ->setTargetType(ConstType::searchOrCreate(*getGeneralListRobinType()));
		ConversionTableSingleton::getInstance()->registerConversion(htogenerallistconst);
	}



	if(!isHyperGeneric())
	{

		/*
		 * Generating a conversion from an empty list to this type.
		 */
		{
			Handle<Conversion> hfromemptylist(new TrivialConversion);
			hfromemptylist    ->setSourceType(getEmptyListRobinType());
			hfromemptylist    ->setTargetType(selfHandle);
			ConversionTableSingleton::getInstance()->registerConversion(hfromemptylist);
		}

		{
			Handle<Conversion> hfromemptylistconst(new TrivialConversion);
			hfromemptylistconst    ->setSourceType(ConstType::searchOrCreate(*getEmptyListRobinType()));
			hfromemptylistconst    ->setTargetType(selfConstHandle);
			ConversionTableSingleton::getInstance()->registerConversion(hfromemptylistconst);
		}
	}
}


Handle<ListRobinType> ListRobinType::create_new(Handle<RobinType> elements_type)
{
	ListRobinType *self = new ListRobinType(elements_type);
	Handle<ListRobinType> h_self = self->get_handler();

	// m_refcount was first set to 1 to make sure the class
	// is not deleted till we return the handler, but now
	// that we have a handler we are safe
	*self->m_refcount -= 1;
	return h_self;
}

ListRobinType::~ListRobinType()
{

}

string ListRobinType::getTypeName() const
{
	return "PyList of " + m_elements_type->getTypeName();
}


/**
 * Find the list type for an element or return NULL if it does not
 * exist.
 */
Handle<RobinType> ListRobinType::findListType(const Handle<RobinType> &element)
{
	if(!element) {
		return getEmptyListRobinType();
	}

	static Implementation &imp = *Implementation::get();

	Type2TypeMap::iterator found = imp.mapTypeToItsList.find(element);
	if(found!= imp.mapTypeToItsList.end()) {
		return found->second;
	} else {
		return Handle<RobinType>(0);
	}
}


bool ListRobinType::isHyperGeneric() const
{
	return m_elements_type->isHyperGeneric();
}


Handle<RobinType> ListRobinType::listForSpecificElements(const Handle<RobinType> &element)
{
	//Try to see if the list is cached
	{
		Handle<RobinType> list_cast = findListType(element);
		if(list_cast) {
			return list_cast;
		}
	}


	static Implementation & imp = *Implementation::get();
	Handle<ListRobinType> list = create_new(element);
	Handle<RobinType> list_cast = static_hcast<RobinType>(list);
	imp.mapTypeToItsList.insert(make_pair(element,list_cast));
	dbg::traceRegistration << "Added type " << *list << dbg::endl;
	dbg::IndentationGuard guard(dbg::trace);
	list->initializeConversions();

	return list_cast;
}


}//namespace Robin::Python
}//namespace Robin

