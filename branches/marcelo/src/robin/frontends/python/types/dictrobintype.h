/*
 * dictrobintype.h
 *
 *  Created on: Jan 28, 2010
 *      Author: Marcelo Taube
 */

#ifndef ROBIN_FRONTENDS_PYTHON_TYPES_DICTROBINTYPE_H_
#define ROBIN_FRONTENDS_PYTHON_TYPES_DICTROBINTYPE_H_

#include <robin/reflection/robintype.h>
#include <pattern/handle.h>


namespace Robin {
namespace Python {

/**
 * A RobinType which represents a PyDict with
 * a fixed type for its key and for its value elements.
 */
class DictRobinType : public RobinType
{
public:

	inline Handle<DictRobinType> get_handler()
	{
		return Handle<DictRobinType>(this,this->m_refcount);
	}
	inline Handle<DictRobinType>  get_handler() const //some day will have to add a const handler for const correctness
	{
		return Handle<DictRobinType>(const_cast<DictRobinType *>(this),this->m_refcount);
	}


	/**
	 * Destructor
	 */
	virtual ~DictRobinType();

	/**
	 *  The name of the type, can be used for debug prints.
	 */
	 virtual std::string getTypeName() const;

	 /**
	  * A dictionary is hypergeneric depending on their
	  * key type and value type.
	  */
	 virtual bool isHyperGeneric() const;

	 /**
	  * Will return the RobinType of dicts whose key and values are of
	  * some specific RobinType.
	  * If the searched type does not exist, it will create it.
	  * @param element The RobinType of the elements or null if the
	  * 				is empty
	  */
	 static Handle<RobinType> dictForSpecificKeyAndValues(const Handle<RobinType> &key,
													   const Handle<RobinType> &value);


	 /*
	  * Returns the RobinType which is the superset of all the dict types
	  * It can be used when we need to register conversions for all the
	  * types of dicts.
	  *
	  * Notice: Registering conversions for such a general type can
	  * 		greatly slow the conversion resolution process.
	  */
	 static Handle<RobinType> getGeneralDictRobinType();

	 /*
	  * Returns the RobinType of an empty dict.
	  * It is the subset of all the dict types.
	  * There is only one value for this type and it is '{}'
	  *
	  */
	 static Handle<RobinType> getEmptyDictRobinType();


protected:


	 static Handle<RobinType> findDictType(const Handle<RobinType> &key, const Handle<RobinType> &value);

	 /*
	  * @name Factory functions
	  */
	 //@{
		 static Handle<DictRobinType> create_new(const Handle<RobinType> &key, const Handle<RobinType> &value);
	 //@}

	/**
	 * @name Constructors
	 */
	//@{

	/*
	 * @param key A type for the keys of the dictionary.
	 * @param value A type for the values of the dictionary
	 */
	DictRobinType(const Handle<RobinType> &key, const Handle<RobinType> &value);

	//@}


	void initializeConversions();

	/**
	 * The RobinType of the keys.
	 */
	Handle<RobinType> m_keys_type;


	/**
	 * The RobinType of the values.
	 */
	Handle<RobinType> m_values_type;

	/**
	 * Internal fields of DictRobinType
	 */
	class Implementation;


	friend class DictConversionProposer;
};








}//namespace Robin::Python
}//namespace Robin


#endif /* ROBIN_FRONTENDS_PYTHON_TYPES_DICTROBINTYPE_H_ */
