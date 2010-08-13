// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * robintype.h
 *
 * @par TITLE
 * Argument Declaration
 *
 * Declares the Argument class, which is the basic building
 * block of prototypes in the reflection. Unlike the
 * <classref>Adapter</classref> (which is an interface exported by the
 * Frontends Framework), the RobinType is a generic object and it's
 * independant of the scripting environment.
 */

#ifndef ROBIN_RobinType_H
#define ROBIN_RobinType_H

// STL includes
#include <vector>
#include <exception>
#include <ostream>

// Pattern includes
#include <pattern/handle.h>
#include <pattern/map.h>
#include <pattern/assoc_heap.h>

// Package includes
#include "argumentsbuffer.h"
#include "types.h"
#include "conversion.h"
#include "conversiontable.h"

namespace Robin {

class Class;
class EnumeratedType;
class Adapter;
class RobinTypeAddedObserver;


/**
 * @par ENUM
 * TypeCategory
 */
enum TypeCategory
{
    TYPE_CATEGORY_INTRINSIC,
    TYPE_CATEGORY_EXTENDED,
    TYPE_CATEGORY_USERDEFINED,
    TYPE_CATEGORY_POINTER
};

/**
 * @par ENUM
 * TypeSpec
 */
enum TypeSpec
{
    TYPE_INTRINSIC_INT,
    TYPE_INTRINSIC_UINT,
    TYPE_INTRINSIC_LONG,
    TYPE_INTRINSIC_LONG_LONG,
    TYPE_INTRINSIC_ULONG,
    TYPE_INTRINSIC_ULONG_LONG,
    TYPE_INTRINSIC_CHAR,
    TYPE_INTRINSIC_SCHAR,
    TYPE_INTRINSIC_UCHAR,
    TYPE_INTRINSIC_SHORT,
    TYPE_INTRINSIC_USHORT,
    TYPE_INTRINSIC_LONGLONG,
    TYPE_INTRINSIC_ULONGLONG,
    TYPE_INTRINSIC_FLOAT,
    TYPE_INTRINSIC_DOUBLE,
    TYPE_INTRINSIC_BOOL,
    TYPE_EXTENDED_VOID,
    TYPE_EXTENDED_CSTRING,
	TYPE_EXTENDED_PASCALSTRING,
	TYPE_EXTENDED_ELEMENT,
    TYPE_USERDEFINED_OBJECT,
    TYPE_USERDEFINED_ENUM,
    TYPE_USERDEFINED_INTERCEPTOR,
    TYPE_POINTER,
    TYPE_SPEC_SIZE // this is not a spec, it stores the size of the table
};

/**
 * @par STRUCT
 * Type
 *
 * Represents a basic type (without modifiers).
 */
struct Type
{
    TypeCategory category;
    TypeSpec     spec;
    Handle<Class> objclass;  /* applies when spec==TYPE_USERDEFINED_OBJECT */
    Handle<EnumeratedType> objenum;  /* when spec==TYPE_USERDEFINED_ENUM */
    const char *name; // in other cases it can be used for debugging
    ~Type();
};

class TypeExistanceObservable;
class TypeToWeightMap;
class ConversionTree;



/**
 *  Part of the conversion path search algorithm implemented
 *  in ConversionTable::bestSingleRoute.
 *
 *  When a path was found to a certain RobinType a ConversionProposer
 *  will propose edges from that type to concatenate and reach other
 *  RobinTypes.
 *
 *  A RobinType itself is also a ConversionProposer because
 *  it knows the edges to its neighboring RobinTypes.
 *
 *  The interface can also be used to help the
 *  RobinType to find its neighbors, if the type supports it.
 *
 */
class ConversionProposer
{
public:
	virtual ~ConversionProposer() = 0;

	 /**
	  * Used to find conversion routes to a certain RobinType.
	  * It is used from the method ConversionTable::bestSingleRoute
	  * which implements the Dijkstra shortest path algorithm.
	  *
	  * The method proposeConversionContinuations is called when the
	  * route to a specific RobinType has already been calculated, its task
	  * is to propose how to continue searching for the desired path.
	  *
	  *
	  * Notice that a there are two possibilities for continuations from
	  * a type, const conversions or two-directional conversion which
	  * copies the values back to the original object. The parameter
	  * constConversions decides which kind of conversions to use.
	  *
	  * @param reachedWeight This is the weight of the route from the
	  * 					 source RobinType to the just reached type.
	  * @param bestWeighHeap It is a heap which sorts all the robin types
	  * 					which have been reached by the weights of
	  * 					the routes found to them. It is mentioned in
	  * 				    the Dijkstra algorithm.
	  * 					This parameter will be modified to reflect the
	  * 					new paths found from the edges coming out of this
	  * 					type.
	  * @param constConversions whatever we are building a tree of
	  * 					const conversions (conversions which
	  * 					will not modify the original value).
	  * @param distanceMap The current distance to each RobinType that has been
	  * 					found. Will be modified.
	  * @param previousStepMap The previous edge in the current path to
	  * 						each robintype. Will be modified
	  */
	 virtual void proposeConversionContinuations(
			 const Conversion::Weight &reachedWeight,
			 AssocHeap<Conversion::Weight,const RobinType*> & bestWeightHeap,
			 bool constConversions,
			 TypeToWeightMap & distanceMap,
			 ConversionTree & previousStepMap) const = 0;
};


/**
 * @class RobinType
 * @nosubgrouping
 *
 * Not surprisingly, a RobinType describes a type in robin. We will see later how
 * it is different from a type of the wrapped language(C++ for example)
 * or a type of the user language(python for examples).
 *
 * RobinTypes like C++ types are used to define argument types and
 * to express which type-conversion to do on both implicit or explicit
 * conversions.
 *
 * Currently there is one and only one RobinType object for each type we want
 * to represent. No two RobinTypes will be constructed with the same parameters.
 * This is important to reduce memory use of robin.
 * Because of that comparing the addresses of the RobinType objects
 * might be enough to know if they are the same type (thus no operator== is implemented),
 * in spite of this it is recommended to compare the id of the type which is obtained by
 * the method  RobinType::getID for future compatibility.
 *
 * Differences between original types of the programming language and C++:
 * 1) RobinTypes are like the union of the two languages type systems. A
 * RobinType can represent a C++ class, the low level 'char' type,
 * a python list or a python long.
 *
 * 2) Sometimes a RobinType can represent a dummy type which
 * only exists within robin, provably used as an intermidiate type
 * when doing a conversion.
 *
 * 3) Robin types can sometimes be more refined than the original type
 * they represent.  As an example: The type of a list in python is 'list'
 * however it is important in robin to know which type of elements the
 * list contains so there are RobinTypes which represent list of integers, lists
 * of strings, empty lists,etc.
 *
 * 4)  Another type of refinement from the original type might be the
 * addition of data which shows how the argument is passed to a function:
 * by value, by pointer,etc.
 *
 *
 * Historical note: This class was once called 'TypeOfArgument'.
 *
 * Implementation problems:
 * Currently the class holds fields which are only useful on certain
 * types but are not used or have invalid values on other types.
 * This keeps the class big and complicated, with lots of fields and
 * methods which complicate.
 * It is desirable that in the future the class RobinType will be a
 * minimalistical interface and will be different implementations for
 * each type of types.
 * Example: a class for python types, a class for C types, etc.
 *
 */
class RobinType : public ConversionProposer
{
private:
	//The copy constructor does not exist
	explicit RobinType(const RobinType &);
	// The operator  does not exist
	RobinType &operator=(const RobinType &);
public:
	/**
	 * It specifies whatever a type is a constant reference to
	 * an object.
	 * If a reference is a constant reference it cannot be changed
	 * in a way which will affect the original object.  That is
	 * not only a recommendation but a fact because a reference might
	 * actually be a copy of the object not the object itself
	 * (for example when type conversion is done automatically to
	 * pass a variable).
	 * Non constant references also support conversions but there must
	 * be specific support for non const conversions since the changes
	 * have to be updated back in the original object.
	 *
	 * Notice: Instead of an enum in  the RobinType it might be much
	 *      more adequate to have an extra RobinType called ConstType
	 *      which wraps any type. This class exists and is work in progress
	 *      some of the RobinTypes already use ConstType to represent const
	 *      references to itself, but still work has to be done.
	 */
	enum ConstnessKind { constReferenceKind=1, regularKind=0 };
protected:
    /**
     * @name Constructors
     */

    //@{
	RobinType(TypeCategory category, TypeSpec spec, const char * name, ConstnessKind kind,bool borrowed = false);
    RobinType(TypeCategory category, TypeSpec spec,  ConstnessKind kind ,bool borrowed = false);
    RobinType(Handle<Class>, ConstnessKind kind , bool borrowed = false);
    RobinType(Handle<EnumeratedType>, ConstnessKind kind , bool borrowed = false);

    //@}
public:
	/**
	 * All these functions do the job of the constructor,
	 * but return a handle.
	 * If inheriting from this class, it is important to reimplement
	 * the method create_new completely without using the version from the parent class.
	 * @name Factory functions
	 */
	//@{
	static Handle<RobinType> create_new(TypeCategory category, TypeSpec spec, const char * name, ConstnessKind kind ,bool borrowed = false);
	static Handle<RobinType> create_new(TypeCategory category, TypeSpec spec, ConstnessKind kind , bool borrowed = false);
	static Handle<RobinType> create_new(Handle<Class>, ConstnessKind kind , bool borrowed = false);
	static Handle<RobinType> create_new(Handle<EnumeratedType>, ConstnessKind kind , bool borrowed = false);
	//@}

    virtual ~RobinType();


	/**
	 * Get a handle to this object.
	 * The handle works coordinated with the rest of the handles created here.
	 * @name Handle to self getters.
	 */
    //@{
		inline Handle<RobinType> get_handler()
		{
			return Handle<RobinType>(this,this->m_refcount);
		}
		inline Handle<RobinType>  get_handler() const //some day will have to add a const handler for const correctness
		{
			return Handle<RobinType>(const_cast<RobinType *>(this),this->m_refcount);
		}
protected:
		/**
		 * A reference counter to 'this', it reflects how many Handles
		 * point to this object.
		 *
		 * It is used so this class can generate handlers to itself
		 *
		 * Notice1: that memory used by m_refcount does not need to be released (deleted)
		 * from this class, Handle itself will delete it when it will be needed.
		 *
		 * Notice2: a constructor of this class will initialize the value of
		 *         the reference counting to 1
		 *        and at the end of the function create_new the value has to
		 *        be decreased by one (to make sure the object will not be
		 *        released accidentally during the construction).
		 */
		mutable int *m_refcount;
public:
	//@}


    /**
     * @name Access
     */

    //@{
    Type basetype() const;
	bool isReference() const;
    bool isBorrowed() const;

    /**
     * Hyper generic types are used to represent objects which
     * belong to several subtypes, for example:
     *  - the object [] belongs to 'list<int>' and 'list<string>'
     *     it will be assigned type 'emptylist'
     *  - the object {} belongs to 'dict<int, int>' and 'dict<int, string>'
     *     it will be assigned hyper-generic type 'emptydict'
     *  - the object [[]] belongs to 'list< list< int > >' and list< list<string > >'
     *     it will be assigned hyper-generic type 'list<emptylist>'
     *
     *  The objective of knowing is a type is hypergeneric is to
     *  minimize the possibility of infinite loops in the conversion
     *  path search algorithm.
     *
     *  There should be no conversion edges entering hypergeneric types, that
     *  way there is less danger of generating some complex loop of
     *  involving lists/dicts of different levels
     */
    virtual bool isHyperGeneric() const;

    /**
     * Is a pointer to this type.
     * Notice this is part of a future development.
     * It implements recursive pointer functionality.
     * It is not related to the current+stable pointer to class
     * mechanism.
     */
	Handle<RobinType> pointer() const;
    //@}

    /**
     * @name Translation
     *
     * Methods that perform Active Translation
     * by forwarding the call to the Adapter.
	 * These operations help connect between the Internal
	 * Reflection and the Frontends Framework; the RobinType stores
	 * an <classref>Adapter</classref> which is implemented by the
	 * frontend, and uses it to perform the translations.
	 */

    //@{
    void assignAdapter(Handle<Adapter> adapter);

    scripting_element get(basic_block rval);
    void put(ArgumentsBuffer& argsbuf, scripting_element value);
	//@}


	/**
	 * Prints the type parameter in a human readable form.
	 */
	friend  std::ostream &operator<<(std::ostream &out, const RobinType &output);


	/**
	 * Returns a constant number which is different for each RobinType.
	 * Notice that the address of RobinType is also a distinctive
	 * number for a RobinType, but there are two advantages for getID():
	 * 	- Since the id increases only by one (and the address has alignment),
	 *    it is better for using in arrays and hashes
	 *  - getID is less implementation dependent (suppose that in the
	 *    future RobinTypes could be passed by value, the address would
	 *    change but the id wont).
	 */
	inline size_t getID() const{
		return m_id;
	}

	/**
	 *  The name of the type, can be used for debug prints.
	 */
	 virtual std::string getTypeName() const;

	/**
	 * It provides the conversion proposed which will be used to wrap this
	 * type.
	 *
	 * Should be called only one time
	 */
	 void setConversionProposer(const Handle<ConversionProposer> &proposer);

	 /**
	  * Implements the basic method of interface ContinuationsProposer.
	  *
	  * In its basic implementation it will simply consider the registered
	  * edges in the conversion table and concatenate their weight to the
	  * weight calculated for 'this'.
	  *
	  */
	 virtual void proposeConversionContinuations(
			 const Conversion::Weight &reachedWeight,
			 AssocHeap<Conversion::Weight,const RobinType*> & bestWeightHeap,
			 bool constConversions,
			 TypeToWeightMap & distanceMap,
			 ConversionTree & previousStepMap) const;

	 /**
	  * @name Working with constant references
	  */
	 //@{
	      /**
	       * Read the documentation of ConstnessKind to understand
	       * about this method.
	       */
          ConstnessKind isConstant() const;

          /**
           * It announces if/when the const type for this RobinType
           * is added.
           */
          Handle<TypeExistanceObservable> m_constTypeAdditionAnnouncer;

	 //@}
protected:
    Type m_basetype;
    bool m_borrowed;
    std::vector<int> m_array_dimensions;

    Handle<Adapter> m_adapter;

	// - pointer
	mutable Handle<RobinType> m_cache_pointer;

	/**
	 * A number which is different for each RobinType
	 */
	const size_t m_id;

	/*
	 * Whatever values of this type can be modified.
	 * Read documentation of isConstant()
	 */
	ConstnessKind m_constType;


private:
	/**
	 * A number used to calculate m_id.
	 * It also shows how many RobinTypes objects were
	 * created.
	 */
	static size_t RobinTypes_counter;

	Handle<ConversionProposer> m_proposer;

	void addTrivialConversions(
			ConversionTable::AdjacencyList &list) const;
};

/*************************************************************
 *
 *    Derived types from RobinType (vectors, maps,etc)
 *
 *************************************************************/

typedef std::vector<Handle<RobinType> > RobinTypes;

/**
 * It maps from robin types to the weight needed to reach them.
 */
class TypeToWeightMap : public std::map<const RobinType*,Conversion::Weight>
{
private:
	typedef std::map<const RobinType*,Conversion::Weight> super;
public:
	using typename super::iterator;
	 /*
	  * It updates that a new path was found for an element, but we
	  * only want to update the map if there is no previous weight
	  * which is lighter than the new one.
	  * Also it refuses to update weights that are higher than the
	  * maximum C++ allowed weights (see Conversion::Weight::isPossible)
	  *
	  * @returns true if an update was done
	  */
	 inline bool updateWeightIfBetter(const RobinType*element, const Conversion::Weight &value)
	 {
		 iterator previous_it = find(element);
		 if(value.isPossible() and
				 (previous_it == end() ||  value < previous_it->second ))
		 {
			 (*this)[element] = value;
			 return true;
		 }
		 return false;
	 }
};


/**
 * Prints the type parameter in a human readable form.
 */
std::ostream &operator<<(std::ostream &out, const RobinType &output);

/**
 * @class UnsupportedInterfaceException
 * @nosubgrouping
 *
 * Thrown when trying to perform translations using a
 * <classref>RobinType</classref> which does not have an
 * <classref>Adapter</classref> associated with it.<br />
 * Also, this excpetion is thrown by a frontend when the type of an
 * element cannot be detected, or when an <classref>Adapter</classref>
 * is not available for this object type.
 */
class UnsupportedInterfaceException : public CannotCallException
{
public:
    const char *what() const throw() { return "unsupported interface"; }
};

} // end of namespace Robin
#include "typeexistanceobservable.h"

#endif
//@}
