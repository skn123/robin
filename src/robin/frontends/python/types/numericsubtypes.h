/*
 * numericsubtypes.h
 *
 *  Created on: Jan 25, 2010
 *      Author: Marcelo Tauube
 */

#ifndef ROBIN_FRONTENDS_PYTHON_TYPES_NUMERICSUBTYPES_H_
#define ROBIN_FRONTENDS_PYTHON_TYPES_NUMERICSUBTYPES_H_

#include <robin/reflection/robintype.h>
#include "../pyhandle.h"

namespace Robin {
namespace Python {


/**
 * A RobinType which represents a subtype of a Python numeric type,
 * either python long or python int (c-long),but which is bounded to a range.
 * The existence of this subtypes is important in order to know if the number
 * can be converted to primitive C types.
 * First of all there is need to know if a number is positive to know
 * if it can be converted to an unsigned type or not.
 * Then it is also important to know which is the smallest type that
 * can hold it.
 * Numbers will be separated to specific ranges, such that all the
 * numbers of the range can be represented in the exactly the same
 * types.
 * Because of that, types of the kind BoundedNumericRobinType
 * will be non intersecting, each number belongs to exactly one range.
 *
 * To describe (determine) the range three characteristics will be used.
 * First if the range is positive or negative (there will be no ranges
 * that crosses 0). Also it is important to know what is the amount of digits
 * (bits not used for the sign) which are needed to represent the biggest
 * number of the range and the smallest number of the range.
 *
 * The Ctypes considered to set bounds of the ranges are short, ushort,
 * int, uint, long, ulong, longlong, ulonglong.
 *
 * Thus there will be a range of the numbers which will fit in a ulong but
 * do not fit in a long.
 *
 * Some examples will be now shown, all of them assume the compiler is
 * gcc 32 bits.
 *
 * Example 1: the number 3 belongs to the range [0, (1<<15)-1] because
 * it fits in an 'short' and is a positive number.
 *
 * Example 2: the number 2147483648L == 1<<31 belongs to the range [1<<31, (1<<32)-1]
 *  because it fits in an 'unsigned int' but not in an 'int'
 *
 * Example 3: the number -2147483650L == -(1<<31)-2 belongs to the range [-(1<<63), -(1<<31)-1]
 * because it fits in a 'long long' but not in a 'long' (assuming 32 bits compilation)
 *
 */
class BoundedNumericRobinType : public RobinType
{
public:

	/**
	 * Destructor
	 */
	virtual ~BoundedNumericRobinType();

	/**
	 *  The name of the type, can be used for debug prints.
	 */
	 virtual std::string getTypeName() const;




	 /*
	  * @name Factory functions
	  */
	 //@{

		 /**
		  * Searches for BoundedNumericRobinType to which the parameter
		  * num belongs.
		  */
		 static Handle<RobinType> giveTypeForNum(PyReferenceBorrow<PyIntObject> num);

		 /**
		  * Searches for BoundedNumericRobinType to which the parameter
		  * num belongs.
		  */
		 static Handle<RobinType> giveTypeForNum(PyReferenceBorrow<PyLongObject> num);

		/*
		 * Does the job of the constructor, but returns a handle.
		 * If inheriting from this class, it is important to reimplement
		 * the method create_new completely without using the version from the parent class.
		 *
		 * @param digits digits needed to represent the max value for this range
		 *
		 * @param positive whatever the numbers are positive number or not
		 */
		 static Handle<BoundedNumericRobinType> create_new(long min_digits, long max_digits, bool positive, Handle<RobinType> superType);
	 //@}


protected:



	/**
	 * @name Constructors
	 */
	//@{

	/*
	 * @param digits digits needed to represent the max value for this range
	 *
	 * @param positive whatever the numbers are positive number or not
	 */
	BoundedNumericRobinType(long min_digits, long max_digits, bool positive, Handle<RobinType> superType);

	//@}

	void initializeConversions();

	/**
	 * The amount of digits to represent the smallest absolute value of the
	 * range.
	 */
	long m_minDigits;

	/**
	 * The amount of digits to represent the biggest absolute value of the
	 * range.
	 */
	long m_maxDigits;

	/**
	 * Whatever this range is in the positive side of the
	 * number line.
	 */
	bool m_positive;

	/**
	 * a string representing the maximum value in the range
	 */
	std::string m_max;

	/**
	 * a string representing the minimum value in the range.
	 */
	std::string m_min;

	/**
	 * The type we are subtyping, examples: ArgumentLong or ArgumentPythonLong
	 */
	Handle<RobinType> m_superType;
};


}// end of namespace Robin::Python

}// end of namespace Robin



#endif /* ROBIN_FRONTENDS_PYTHON_TYPES_NUMERICSUBTYPES_H_ */
