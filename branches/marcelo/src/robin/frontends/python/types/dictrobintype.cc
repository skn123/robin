/*
 * dictrobintype.cc
 *
 *  Created on: Jan 28, 2010
 *      Author: Marcelo Taube
 */
#include <Python.h>
#include "../port.h"
#include "dictrobintype.h"
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/memorymanager.h>
#include <robin/reflection/conversiontree.h>
#include <robin/reflection/fundamental_conversions.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/const.h>
#include <robin/debug/assert.h>
#include <robin/frontends/python/pyhandle.h>
using namespace std;

namespace Robin {
namespace Python {


/**
 * A RobinType which represents an empty python dict.
 */
class EmptyPythonDictType : public RobinType
{
public:

	/**
	 * Destructor
	 */
	virtual ~EmptyPythonDictType()
	{

	}

	/**
	 *  The name of the type, can be used for debug prints.
	 */
	 virtual string getTypeName() const
	 {
		 return "empty PyDict";
	 }

	 /*
	  * @name Factory functions
	  */
	 //@{
		 static Handle<EmptyPythonDictType> create_new()
		{
			EmptyPythonDictType *self = new EmptyPythonDictType();
			Handle<EmptyPythonDictType> h_self = ::static_hcast<EmptyPythonDictType>(self->get_handler());

			// m_refcount was first set to 1 to make sure the class
			// is not deleted till we return the handler, but now
			// that we have a handler we are safe
			*self->m_refcount -= 1;
			return h_self;
		}
	 //@}

	virtual bool isHyperGeneric() const
	{
		return true;
	}
protected:
	/**
	 * @name Constructors
	 */
	//@{

		 EmptyPythonDictType()
			 : RobinType(TYPE_CATEGORY_EXTENDED, TYPE_EXTENDED_VOID, "PyDict", RobinType::regularKind)
		 {

		 }

	//@}

};


/**
 * A RobinType which represents any python dict which its
 * elements might not be specific of a certain type.
 */
class GeneralPythonDictType : public RobinType
{
public:

	/**
	 * Destructor
	 */
	virtual ~GeneralPythonDictType()
	{

	}

	/**
	 *  The name of the type, can be used for debug prints.
	 */
	 virtual string getTypeName() const
	 {
		 return "general PyDict";
	 }

	 /*
	  * @name Factory functions
	  */
	 //@{
		 static Handle<GeneralPythonDictType> create_new()
		{
			GeneralPythonDictType *self = new GeneralPythonDictType();
			Handle<GeneralPythonDictType> h_self = ::static_hcast<GeneralPythonDictType>(self->get_handler());

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

		 GeneralPythonDictType()
			 : RobinType(TYPE_CATEGORY_EXTENDED, TYPE_EXTENDED_VOID, "PyDict", RobinType::regularKind)
		 {

				 Handle<RobinType> selfHandle = this->get_handler();

				/*
				 * Generating a conversion from an empty dict type to this type
				 */
				Handle<Conversion> hfromemptydict(new TrivialConversion);
				hfromemptydict    ->setSourceType(DictRobinType::getEmptyDictRobinType());
				hfromemptydict    ->setTargetType(selfHandle);
				ConversionTableSingleton::getInstance()->registerConversion(hfromemptydict);

				/*
				 * Generating a const conversion from a const empty dict type to this type
				 */
				{
					Handle<Conversion> hfromconstemptydict(new TrivialConversion);
					hfromconstemptydict    ->setSourceType(ConstType::searchOrCreate(*DictRobinType::getEmptyDictRobinType()));
					hfromconstemptydict    ->setTargetType(ConstType::searchOrCreate(*selfHandle));
					ConversionTableSingleton::getInstance()->registerConversion(hfromconstemptydict);
				}

				/*
				 * Generating the conversion to scripting_element, for
				 * functions which knows how to handle python objects directly
				 */
				Handle<Conversion> hdict2element(new TrivialConversion);
				hdict2element    ->setSourceType(selfHandle);
				hdict2element    ->setTargetType(ArgumentScriptingElementNewRef);
				ConversionTableSingleton::getInstance()->registerConversion(hdict2element);


		 }

	//@}

};

/**
 * It is the traits for a Map with key a pair of a robin types as
 * a key.
 * The value is not specified, so to properly use this traits you
 * need to inherit from it and specify the value.
 */
struct TraitsForTwoTypeKeys {
	typedef std::pair<Handle<RobinType> , Handle<RobinType> > Key;
private:
	inline static size_t firstId(const Key &key) {
		return key.first->getID();
	}
	inline static size_t secondId(const Key &key) {
		return key.second->getID();
	}
public:
	struct KeyCompareFunctor
	{
		bool operator()(const Key& key1,
						const Key& key2) const
		{
			return firstId(key1) < firstId(key2) or
					(firstId(key1) == firstId(key2) &&
						secondId(key1) < secondId(key2));
		}
	};

	struct KeyIdentityFunctor
	{
		bool operator()(const Key& key1,
						const Key& key2) const
		{
			return firstId(key1) == firstId(key2) &&
					secondId(key1) == secondId(key2);
		}
	};

	struct KeyHashFunctor
	{
		size_t operator()(const Key& key) const
		{
			return  firstId(key) + 3* secondId(key);
		}
	};
};

/**
 * Traits structure for Pattern::Map
 */
struct TraitsForTwoTypes2Type  : public TraitsForTwoTypeKeys
{
			typedef Handle<RobinType> Value;
};

typedef Pattern::Map<TraitsForTwoTypes2Type> TwoTypes2Type;





class DictRobinType::Implementation {
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


	TwoTypes2Type mapKeyValueToDict;

};

/**
 * It is a kind of conversion between dicts which internally only
 * does an element by element conversion, and the resulting dict is
 * a constant reference, which means that modifications in the new dict
 * will not affect the original one.
 * The internal conversion is specified by two Conversion objects.
 * The type of the objects in the dict must be from a const kind, that is
 * because the values on the original object will not be updated.
 */
class ConstDictComposedConversion : public Conversion {
public:
	/**
	 * @name Constructors
	 */

	//@{
	ConstDictComposedConversion(Handle<ConversionRoute> keyConversion,
							 Handle<ConversionRoute> valueConversion)
		: m_keyConversion(keyConversion), m_valueConversion(valueConversion)
	{
		Conversion::Weight w_key = keyConversion->totalWeight();
		Conversion::Weight w_value = valueConversion->totalWeight();
		if(w_key<w_value) {
			setWeight(w_key);
		} else {
			setWeight(w_value);
		}
		assert_true(keyConversion->hasOnlyConstantConversions());
		assert_true(valueConversion->hasOnlyConstantConversions());
	}
	//@}

	/**
	 * @name Access
	 */
	//@{
	virtual bool isZeroWorkConversion() const
	{
		return m_keyConversion->isZeroWorkConversionRoute() && m_valueConversion->isZeroWorkConversionRoute();
	}
	//@}

	/**
	 * @name Activity
	 */

	//@{
	virtual scripting_element apply(scripting_element element) const
	{
		if(isZeroWorkConversion()) {
			Py_XINCREF((PyObject *)element);
			return element;
		}

		PyObject *object = (PyObject *)element;
		assert_true(PyObject_TypeCheck(object,&PyDict_Type));
		PyReferenceSteal<PyObject> newDict(PyDict_New());

		PyObject *key, *value;
		Py_ssize_t pos = 0;

		GarbageCollection garbageCollector;
		while (PyDict_Next(object, &pos, &key, &value)) {
			PyReferenceCreate<PyObject> key_converted(
					(PyObject*)m_keyConversion->apply(key,garbageCollector));
			PyReferenceCreate<PyObject> value_converted(
									(PyObject*)m_valueConversion->apply(value,garbageCollector));
			// Notice that this call DOES NOT release the previous
			// reference at position i in the dict.
			int ret =PyDict_SetItem(newDict.pointer(),key_converted.pointer(), value_converted.pointer());
			if(ret != 0) {
				throw std::runtime_error("failed to automatically convert dictionaries");
			}
		}

		return newDict.release();

	}

	//@}
protected:
	Handle<ConversionRoute> m_keyConversion;
	Handle<ConversionRoute> m_valueConversion;
};


/**
 * It is a kind of conversion between dicts which
 * does an element by element conversion in-place, and the resulting dict is
 * a non-const reference which actually points to the same dict as the original.
 * The internal conversion is specified by another Conversion object.
 * The type of the objects in the dict must be from a const kind, that is
 * because the values on the original object will not be updated, they will
 * only be replaced.
 */
class NonConstDictComposedConversion : public Conversion {
public:
	/**
	 * @name Constructors
	 */

	//@{
	NonConstDictComposedConversion(Handle<ConversionRoute> keyConversion,
			 Handle<ConversionRoute> valueConversion)
	: m_keyConversion(keyConversion), m_valueConversion(valueConversion)
	{
		Conversion::Weight w_key = m_keyConversion->totalWeight();
		Conversion::Weight w_value = m_valueConversion->totalWeight();
		if(w_key<w_value) {
			setWeight(w_key);
		} else {
			setWeight(w_value);
		}
		assert_true(keyConversion->hasOnlyConstantConversions());
		assert_true(valueConversion->hasOnlyConstantConversions());
	}
	//@}

	/**
	 * @name Access
	 */
	//@{
	virtual bool isZeroWorkConversion() const
	{
		return m_keyConversion->isZeroWorkConversionRoute() && m_valueConversion->isZeroWorkConversionRoute();
	}
	//@}

	/**
	 * @name Activity
	 */

	//@{
	virtual scripting_element apply(scripting_element element) const
	{
		if(isZeroWorkConversion()) {
			Py_XINCREF((PyObject *)element);
			return element;
		}

		PyObject *object = (PyObject *)element;
		assert_true(PyObject_TypeCheck(object,&PyDict_Type));

		Py_ssize_t len = PyDict_Size(object);
		typedef std::vector<PyReferenceCreate<PyObject> > ElemVec;
		ElemVec keys_converted(len);
		ElemVec values_converted(len);
		ElemVec::iterator key_end = keys_converted.end();
		ElemVec::iterator value_end = values_converted.end();


		PyObject *key_old, *value_old;
		Py_ssize_t pos = 0; // pos is used by PyDictNext as an index
					//  (please notice it does not advance linearly)

		// Extract values from the original dict.
		ElemVec::iterator key_it = keys_converted.begin();
		ElemVec::iterator value_it = values_converted.begin();
		GarbageCollection garbageCollector;
		while(PyDict_Next(object, &pos, &key_old, &value_old)) {
			PyReferenceCreate<PyObject> newKey((PyObject*)m_keyConversion->apply(key_old,garbageCollector));
			*key_it = newKey;
			PyReferenceCreate<PyObject> newValue((PyObject*)m_valueConversion->apply(value_old,garbageCollector));
			*value_it = newValue;
			key_it++;
			value_it++;
		}
		assert_true(key_it == key_end);
		assert_true(value_it == value_end);


		//Add the values to a new dictionary to make sure they
		// can be added to a dictionary
		PyReferenceSteal<PyObject> newDict(PyDict_New());

		key_it = keys_converted.begin();
		value_it = values_converted.begin();
		for(;key_it != key_end;key_it++,value_it++)
		{
			int ret =PyDict_SetItem(newDict.pointer(),
							key_it->pointer(),
							value_it->pointer());
			if(ret != 0) {
				throw std::runtime_error("failed to automatically convert dictionaries");
			}
		}

		//Update the original dictionary
		PyDict_Clear(object);
		int ret = PyDict_Update(object,newDict.pointer());
		if(ret != 0) {
			throw std::runtime_error("failed to automatically convert dictionaries, PyDict_Update returned an error");
		}
		Py_XINCREF(object);
		return element;
	}

	//@}
protected:
	Handle<ConversionRoute> m_keyConversion;
	Handle<ConversionRoute> m_valueConversion;
};

Handle<RobinType> DictRobinType::getGeneralDictRobinType()
{
	static Handle<RobinType> generalPythonDict
		= ::static_hcast<RobinType>(GeneralPythonDictType::create_new());
	return generalPythonDict;

}

Handle<RobinType> DictRobinType::getEmptyDictRobinType()
{
	static Handle<RobinType> emptyPythonDict
		= ::static_hcast<RobinType>(EmptyPythonDictType::create_new());
	return emptyPythonDict;
}


DictRobinType::DictRobinType(const Handle<RobinType> &key,const Handle<RobinType> &value)
	: RobinType(TYPE_CATEGORY_EXTENDED, TYPE_EXTENDED_VOID, "PyDict", regularKind),
	m_keys_type(key), m_values_type(value)
{

}



/**
 * Generates conversions on the fly for the conversion
 * search algorithm.
 *
 * The conversions will convert from a specific dictType
 * or from a constDictType.
 */
class DictConversionProposer : public ConversionProposer
{
private:

	DictRobinType *m_dictType;
	ConstType *m_constDictType;
	bool m_isConstDictProposer;
public:
	/**
	 * It constructs a DictConversionProposer for a specific type of
	 * dict or constant type of dict.
	 * Notice that the two types have to be passed but the conversion
	 * will be from both types.
	 *
	 * @param dictType
	 * @param constDictType
	 * @param constDictConverter whatever this proposer will be used
	 * 							to generate conversions from the dictType
	 * 							or from the constDictType
	 */
	DictConversionProposer(DictRobinType* dictType,ConstType* constDictType,bool constDictConverter);

	virtual void proposeConversionContinuations(
			 const Conversion::Weight &reachedWeight,
			 AssocHeap<Conversion::Weight,const RobinType*> & bestWeightHeap,
			 bool constConversions,
			 TypeToWeightMap & distanceMap,
			 ConversionTree & previousStepMap) const;

};

DictConversionProposer::DictConversionProposer(DictRobinType *dictType,
		ConstType *constDictType, bool isConstDictProposer)

	:	m_dictType(dictType), m_constDictType(constDictType), m_isConstDictProposer(isConstDictProposer)
{

}

void DictConversionProposer::proposeConversionContinuations(
		 const Conversion::Weight &reachedWeight,
		 AssocHeap<Conversion::Weight,const RobinType*> & bestWeightHeap,
		 bool constConversions,
		 TypeToWeightMap & distanceMap,
		 ConversionTree & previousStepMap) const
{

	if(m_isConstDictProposer) {
		// cannot do a nonconst conversion from a const dict
		if(!constConversions){
			return;
		}
	} else {
		if(constConversions) {
			// this proposer wont add any const conversion from a non
			// const dict
			// (of course the regular conversions in the conversion table
			// will still happen).
			return;
		}
	}

	 Handle<RobinType> sourceType;
	 if(m_isConstDictProposer)
	 {
		 sourceType = m_constDictType->get_handler();
	 } else {
		 sourceType = m_dictType->get_handler();
	 }

	 /*
	  * If the previous step to us was not a dict type then
	  * add all the 'dict to dict' conversions.
	  */
	 if(   previousStepMap.previousConversionOfKind<ConstDictComposedConversion>(sourceType)
		|| previousStepMap.previousConversionOfKind<NonConstDictComposedConversion>(sourceType))
	 {
		return;
	 }

	/*
	 * Now doing an internal type conversion search in the elements
	 */
	 Handle<ConversionTree>   previousStepMapKey;
	 Handle<ConversionTree>   previousStepMapValue;

	 /*
	  * Searching const conversions for elements
	  */
	 ConversionTable *convTable = ConversionTableSingleton::getInstance();
	 {
#		 if ROBIN_DEEP_DEBUG_CONVERSIONS
			 dbg::trace << "Considering internal key conversions to generate dict edges" << dbg::endl;
			 dbg::IndentationGuard guard(dbg::trace);
#		 endif
		 previousStepMapKey = convTable->generateConversionTree(*m_dictType->m_keys_type,0, true);
	 }
	 {
#		 if ROBIN_DEEP_DEBUG_CONVERSIONS
			 dbg::trace << "Considering internal value conversions to generate dict edges" << dbg::endl;
			 dbg::IndentationGuard guard(dbg::trace);
#		 endif
		 previousStepMapValue = convTable->generateConversionTree(*m_dictType->m_values_type,0, true);
	 }


	 /*
	  * Go through all the dict types that exist
	  */
	 static DictRobinType::Implementation *imp = DictRobinType::Implementation::get();
	 TwoTypes2Type::const_iterator it = imp->mapKeyValueToDict.begin();
	 TwoTypes2Type::const_iterator end = imp->mapKeyValueToDict.end();
	 for(;it!=end;it++) {
		 Handle<RobinType> keyTargetType = it->first.first;
		 Handle<RobinType> valueTargetType = it->first.second;
		 Handle<RobinType> dictTargetType;
		 if(m_isConstDictProposer)
		 {
				dictTargetType = ConstType::searchOrCreate(*it->second);
		 } else {
				dictTargetType = it->second;
		 }

		 /*
		  * Check if the key and value types can be reached, if they can
		  * then add the dict conversion too.
		  */
		 ConversionTree::iterator keyDistance_it = previousStepMapKey->find(keyTargetType);
		 ConversionTree::iterator valueDistance_it = previousStepMapValue->find(valueTargetType);
		 if( keyDistance_it != previousStepMapKey->end()
		  && valueDistance_it != previousStepMapValue->end())
		 {
			Conversion::Weight sum = reachedWeight + std::max(keyDistance_it->second->weight(),valueDistance_it->second->weight());
			Handle<RobinType> targetType;

			if (distanceMap.updateWeightIfBetter(dictTargetType.pointer(),sum))
			{
				Handle<Conversion> newEdge;
				Handle<ConversionRoute> keyRoute = previousStepMapKey->generateRouteTo(keyTargetType);
				Handle<ConversionRoute> valueRoute = previousStepMapValue->generateRouteTo(valueTargetType);
				if(m_isConstDictProposer) {
					newEdge = Handle<Conversion>(
							new ConstDictComposedConversion(keyRoute,valueRoute));
				} else {
					newEdge = Handle<Conversion>(
							new NonConstDictComposedConversion(keyRoute,valueRoute));
				};
				newEdge->setSourceType(sourceType);
				newEdge->setTargetType(dictTargetType);
				newEdge->setWeight(sum);
				previousStepMap[dictTargetType] = newEdge /* (u, v) */;
#				if ROBIN_DEEP_DEBUG_CONVERSIONS
					dbg::trace << "Use dynamic edge from: ";
						dbg::trace <<*sourceType;
						dbg::trace << " to " << *dictTargetType <<
						" with price" << sum << dbg::endl;
#				endif
				bestWeightHeap.updateWeight(dictTargetType.pointer(),sum);
			}
		 }
	 }
}


/**
 *  Method which takes care of registering standard conversions from/to
 *  the DictRobinType.
 *
 *  Warning this method might produce a recursive creation of other
 *  DictRobinTypes. The recursion could end up in an infinite loop if
 *  other Dicts try to register conversions to us.
 *  To avoid this the method initializeConversions should only be called
 *  after the DictRobinType has been cached in
 *  DictRobinType::Implementation::mapTypeAndConstnessToItsDict
 */
void DictRobinType::initializeConversions() {
	Handle<DictRobinType> selfHandle = this->get_handler();
	Handle<ConstType> selfConstHandle = ConstType::searchOrCreate(*selfHandle);


	Handle<DictConversionProposer> proposer(
			new DictConversionProposer(selfHandle.pointer(),selfConstHandle.pointer(),false));



	this->setConversionProposer(proposer);

	/*
	 * Generating the conversion to scripting_element, for
	 * function which know how to handle python objects directly
	 */
	{
		Handle<Conversion> htogeneraldict(new TrivialConversion);
		htogeneraldict    ->setSourceType(selfHandle);
		htogeneraldict    ->setTargetType(getGeneralDictRobinType());
		ConversionTableSingleton::getInstance()->registerConversion(htogeneraldict);
	}

	Handle<DictConversionProposer> constProposer(
				new DictConversionProposer(selfHandle.pointer(),selfConstHandle.pointer(),true));
	selfConstHandle->setConversionProposer(constProposer);


	{
		Handle<Conversion> htogeneraldictconst(new TrivialConversion);
		htogeneraldictconst    ->setSourceType(selfConstHandle);
		htogeneraldictconst    ->setTargetType(ConstType::searchOrCreate(*getGeneralDictRobinType()));
		ConversionTableSingleton::getInstance()->registerConversion(htogeneraldictconst);
	}

	if(!isHyperGeneric())
	{

		/*
		 * Generating a conversion from an empty dict to this type.
		 */
		{
			Handle<Conversion> hfromemptydict(new TrivialConversion);
			hfromemptydict    ->setSourceType(getEmptyDictRobinType());
			hfromemptydict    ->setTargetType(selfHandle);
			ConversionTableSingleton::getInstance()->registerConversion(hfromemptydict);
		}

		{
			Handle<Conversion> hfromemptydictconst(new TrivialConversion);
			hfromemptydictconst    ->setSourceType(ConstType::searchOrCreate(*getEmptyDictRobinType()));
			hfromemptydictconst    ->setTargetType(selfConstHandle);
			ConversionTableSingleton::getInstance()->registerConversion(hfromemptydictconst);
		}
	}
}


Handle<DictRobinType> DictRobinType::create_new(const Handle<RobinType> &key,const Handle<RobinType> &value )
{
	DictRobinType *self = new DictRobinType(key,value);
	Handle<DictRobinType> h_self = self->get_handler();

	// m_refcount was first set to 1 to make sure the class
	// is not deleted till we return the handler, but now
	// that we have a handler we are safe
	*self->m_refcount -= 1;
	return h_self;
}

DictRobinType::~DictRobinType()
{

}

string DictRobinType::getTypeName() const
{
	return "PyDict of <" + m_keys_type->getTypeName()+"> to <" + m_values_type->getTypeName() + ">";
}

bool DictRobinType::isHyperGeneric() const
{
	return m_keys_type->isHyperGeneric() || m_values_type->isHyperGeneric();
}

/**
 * Find the dict type for an element or return NULL if it does not
 * exist.
 * @param key The type of the keys. Passing NULL here requests for the
 * 			empty dictionary.
 * @param value The type of the value.
 */
Handle<RobinType> DictRobinType::findDictType(const Handle<RobinType> &key, const Handle<RobinType> &value)
{
	if(!key) {
		return getEmptyDictRobinType();
	}

	static Implementation &imp = *Implementation::get();

	TwoTypes2Type::iterator found = imp.mapKeyValueToDict.find(make_pair(key,value));
	if(found!= imp.mapKeyValueToDict.end()) {
		return found->second;
	} else {
		return Handle<RobinType>(0);
	}
}




Handle<RobinType> DictRobinType::dictForSpecificKeyAndValues(const Handle<RobinType> &key, const Handle<RobinType> &value)
{
	//Try to see if the dict is cached
	{
		Handle<RobinType> dict_cast = findDictType(key,value);
		if(dict_cast) {
			return dict_cast;
		}
	}


	static Implementation & imp = *Implementation::get();
	Handle<DictRobinType> dict = create_new(key,value);
	Handle<RobinType> dict_cast = static_hcast<RobinType>(dict);
	imp.mapKeyValueToDict.insert(make_pair(make_pair(key,value),dict_cast));
	dbg::traceRegistration << "Added type " << *dict << dbg::endl;
	dbg::IndentationGuard guard(dbg::trace);
	dict->initializeConversions();

	return dict_cast;
}


}//namespace Robin::Python
}//namespace Robin

