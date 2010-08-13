/*
 * listrobintype.h
 *
 *  Created on: Jan 28, 2010
 *      Author: Marcelo Taube
 */

#ifndef ROBIN_FRONTENDS_PYTHON_TYPES_LISTROBINTYPE_H_
#define ROBIN_FRONTENDS_PYTHON_TYPES_LISTROBINTYPE_H_

#include <robin/reflection/robintype.h>
#include <pattern/handle.h>

namespace Robin {
namespace Python {



/**
 * A RobinType which represents a PyList with
 * a fixed type for all its elements.
 */
class ListRobinType : public RobinType
{
public:

	inline Handle<ListRobinType> get_handler()
	{
		return Handle<ListRobinType>(this,this->m_refcount);
	}
	inline Handle<ListRobinType>  get_handler() const //some day will have to add a const handler for const correctness
	{
		return Handle<ListRobinType>(const_cast<ListRobinType *>(this),this->m_refcount);
	}

	/**
	 * Destructor
	 */
	virtual ~ListRobinType();

	/**
	 *  The name of the type, can be used for debug prints.
	 */
	 virtual std::string getTypeName() const;


	 /**
	  * Will return the RobinType of lists whose elements are of
	  * some specific RobinType.
	  * If the searched type does not exist, it will create it.
	  * @param element The RobinType of the elements or null if the
	  * 				is empty
	  */
	 static Handle<RobinType> listForSpecificElements(const Handle<RobinType> &element);


	 /*
	  * Returns the RobinType which is the superset of all the list types
	  * It can be used when we need to register conversions for all the
	  * types of lists.
	  *
	  * Notice: Registering conversions for such a general type can
	  * 		greatly slow the conversion resolution process.
	  */
	 static Handle<RobinType> getGeneralListRobinType();

	 /*
	  * Returns the RobinType of an empty list.
	  * It is the subset of all the list types.
	  * There is only one value for this type and it is '[]'
	  *
	  */
	 static Handle<RobinType> getEmptyListRobinType();

	/**
	 * Lists will be hypergeneric depending on their
	 * element type.
	 */
	virtual bool isHyperGeneric() const;


protected:



	 static Handle<RobinType> findListType(const Handle<RobinType> &element);

	 /*
	  * @name Factory functions
	  */
	 //@{
		 static Handle<ListRobinType> create_new(Handle<RobinType> elements_type);
	 //@}

	/**
	 * @name Constructors
	 */
	//@{

	/*
	 * @param elements_type A type of element which represents
	 * 		the type of the internal values of the list.
	 */
	ListRobinType(Handle<RobinType> elements_type);

	//@}


	void initializeConversions();

	/**
	 * The RobinType of the internal element.
	 */
	Handle<RobinType> m_elements_type;


	/**
	 * Internal fields of ListRobinType
	 */
	class Implementation;


	friend class ListConversionProposer;
};





}//namespace Robin::Python
}//namespace Robin


#endif /* ROBIN_FRONTENDS_PYTHON_TYPES_LISTROBINTYPE_H_ */
