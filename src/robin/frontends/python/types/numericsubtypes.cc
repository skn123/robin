/*
 * numericsubtypes.cc
 *
 *  Created on: Jan 25, 2010
 *      Author: Marcelo Taube
 */
#include <Python.h>
#include "numericsubtypes.h"
#include "../pythonadapters.h"
#include <robin/reflection/conversion.h>
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/fundamental_conversions.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <limits>
#include <map>
#include <sstream>

namespace Robin {
namespace Python {


BoundedNumericRobinType::BoundedNumericRobinType(long min_digits, long max_digits, bool positive, Handle<RobinType> superType)
	: RobinType(superType->basetype().category, superType->basetype().spec, superType->basetype().name,superType->isConstant()),
	 m_minDigits(min_digits),  m_maxDigits(max_digits), m_positive(positive),
	m_superType(superType)
{
	if(positive) {
		/*
		 * Now we are on a positive range, means all of it
		 * is bigger or equal to zero, but might even have
		 * a higher minimum.
		 */
		if(m_maxDigits < std::numeric_limits<long>::max())
		{
			std::stringstream maxStr;
			maxStr << "(1<<" << m_maxDigits << ")-1";
			m_max = maxStr.str();
		} else {
			m_max = "+inf";
		}

		if(m_minDigits==0) {
			m_min = "0";
		} else {
			std::stringstream minStr;
			minStr << "(1<<" << m_minDigits << ")";
			m_min = minStr.str();
		}
	} else {
		/*
		 * We are on a negative range, means all of it
		 * is smaller or equal to -1, but might even have
		 * a lower maximum.
		 */
		if(m_maxDigits < std::numeric_limits<long>::max())
		{
			std::stringstream minStr;
			minStr << "-(1<<" << m_maxDigits << ")";
			m_min = minStr.str();
		} else {
			m_min = "-inf";
		}

		if(m_minDigits==0) {
			m_max = "-1";
		} else {
			std::stringstream maxStr;
			maxStr << "-(1<<" << m_minDigits << ")-1";
			m_max = maxStr.str();
		}
	}

	initializeConversions();
}

/**
 * Help function to BoundedNumericRobinType::initializeConversions
 * It creates a conversion from a BoundedNumericRobinType to a intrinsic
 * type only if the conversion can be done, which means only if the subrange
 * fits in the type.
 */
template<typename CPPType>
static inline void _registerConversionFromBoundedType
	(Handle<RobinType> from, bool fromIsPositive, long fromMaxDigits,
	 Handle<RobinType> to, bool toIsUnsigned, Handle<RobinType> superType)
{
	if((fromIsPositive or !toIsUnsigned)
		&& fromMaxDigits <= std::numeric_limits<CPPType>::digits
		&& to!=superType)
	{
		Handle<Conversion> hpylong2numeric(new TrivialConversion);
		hpylong2numeric->setWeight(Robin::Conversion::Weight(0,1,0,0));
		hpylong2numeric->setSourceType(from);
		hpylong2numeric->setTargetType(to);
		ConversionTableSingleton::getInstance()->registerConversion(hpylong2numeric);
	}
}

void BoundedNumericRobinType::initializeConversions() {


	Handle<RobinType> self = this->get_handler();

	/*
	 * Registering a conversion to the wider type we are subtyping.
	 * It is important because a function might get the general type as a
	 * parameter.
	 * Example: We are a subrange of the python longs and the wider class is
	 * 		ArgumentPythonLong
	 */
	Handle<Conversion> tolong(new TrivialConversion); // This does not check properly in 64 bit
	tolong		 ->setSourceType(self);
	tolong       ->setTargetType(m_superType);
	ConversionTableSingleton::getInstance()->registerConversion(tolong);

	//conversions to intrinsic types

	const bool Signed = false;
	const bool Unsigned = true;

	_registerConversionFromBoundedType<long long>
		(self, m_positive, m_maxDigits,
		 ArgumentLongLong, Signed,m_superType);

	_registerConversionFromBoundedType<unsigned long long>
		(self, m_positive, m_maxDigits,
		 ArgumentULongLong, Unsigned,m_superType);

	_registerConversionFromBoundedType<long>
		(self, m_positive, m_maxDigits,
		 ArgumentLong, Signed,m_superType);

	_registerConversionFromBoundedType<unsigned long>
		(self, m_positive, m_maxDigits,
		 ArgumentULong, Unsigned,m_superType);

	_registerConversionFromBoundedType<int>
		(self, m_positive, m_maxDigits,
		 ArgumentInt, Signed,m_superType);

	_registerConversionFromBoundedType<unsigned int>
		(self, m_positive, m_maxDigits,
		 ArgumentUInt, Unsigned,m_superType);

	_registerConversionFromBoundedType<short>
		(self, m_positive, m_maxDigits,
		 ArgumentShort, Signed,m_superType);

	_registerConversionFromBoundedType<unsigned short>
		(self, m_positive, m_maxDigits,
		 ArgumentUShort, Unsigned,m_superType);

}

Handle<BoundedNumericRobinType> BoundedNumericRobinType::create_new(long min_digits, long max_digits, bool positive, Handle<RobinType> superType) {
	BoundedNumericRobinType *self = new BoundedNumericRobinType(min_digits,max_digits,positive,superType);
	Handle<RobinType> ret = self->get_handler();

	// m_refcount was first set to 1 to make sure the class
	// is not deleted till we return the handler, but now
	// that we have a handler we are safe
	*self->m_refcount -= 1;
	return static_hcast<BoundedNumericRobinType>(ret);
}

namespace {
	typedef std::map<unsigned long long, Handle<BoundedNumericRobinType> > UnsignedMap;
	typedef std::map<long long, Handle<BoundedNumericRobinType> > SignedMap;

	void addPositiveSubtype(UnsignedMap &positiveTypes, long min_digits, long max_digits, Handle<RobinType> superType) {
		Handle<BoundedNumericRobinType> sub = BoundedNumericRobinType::create_new(min_digits, max_digits, true, superType);
		unsigned long long max_value;
		if(max_digits<std::numeric_limits<unsigned long long>::digits) {
			// max_value =  2**max_digits -1
			max_value = (((unsigned long long)1)<<max_digits)-1;
		} else {
			max_value = std::numeric_limits<unsigned long long>::max();
		}

		positiveTypes.insert(std::make_pair(max_value,sub));
	}

	void addNegativeSubtype(SignedMap &negativeTypes, long long min_digits, long long max_digits,Handle<RobinType> superType) {
		Handle<BoundedNumericRobinType> sub = BoundedNumericRobinType::create_new(min_digits, max_digits, false, superType);
		long long min_value;
		if(max_digits < std::numeric_limits<long long>::digits) {
			min_value = ((long long)-1)<<max_digits;
		} else {
			min_value = std::numeric_limits<long long>::min();
		}
		negativeTypes.insert(std::make_pair(min_value,sub));
	}
} //end of anonymous namespace

Handle<RobinType> BoundedNumericRobinType::giveTypeForNum(PyReferenceBorrow<PyLongObject> num)
{
	static UnsignedMap positiveTypes;
	static SignedMap negativeTypes;
	static bool initialized=false;

	// First make sure that the type maps are initialized
	if(!initialized) {
		initialized = true;

		addPositiveSubtype(positiveTypes,0,15,ArgumentPythonLong);  //type which fits in a short
		addPositiveSubtype(positiveTypes,15,16,ArgumentPythonLong); //type which fits in a unsigned short
		addPositiveSubtype(positiveTypes,16,31,ArgumentPythonLong); //type which fits in an int (an in a long in 32 platforms)
		addPositiveSubtype(positiveTypes,31,32,ArgumentPythonLong); //type which fits in an unsigned int (an in an unsigned long in 32 platforms)
		addPositiveSubtype(positiveTypes,32,63,ArgumentPythonLong); //type which fits in a longlong (an in a long in 64 platforms)
		addPositiveSubtype(positiveTypes,63,64,ArgumentPythonLong); //type which fits in a unsigned longlong (and in a long in 64 platforms)

		addNegativeSubtype(negativeTypes,0,15,ArgumentPythonLong);  //type which fits in a short
		addNegativeSubtype(negativeTypes,15,31,ArgumentPythonLong); //type which fits in an int (an in a long in 32 platforms)
		addNegativeSubtype(negativeTypes,31,63,ArgumentPythonLong); //type which fits in a longlong (an in a long in 64 platforms)

	} //end of initialization

	//Now try to see which kind of number this is
	unsigned long long value = PyLong_AsUnsignedLongLong((PyObject*)num.pointer());
	if(!PyErr_Occurred()) {
		// The number is a positive value which fits in unsigned long long

		//Search for the robin type
		UnsignedMap::iterator it = positiveTypes.lower_bound(value);
		if(it!= positiveTypes.end()) {
			// We found the value in the map, return it
			return static_hcast<RobinType>(it->second);
		} else {
			throw std::logic_error("Cannot find the RobinType for unsigned long long.");
		}
	} else{
		//the number is a negative value or huge value
		PyErr_Clear();
		long long value_signed = PyLong_AsLongLong((PyObject*)num.pointer());
		if(!PyErr_Occurred()) {
			//The number is negative

			//Search for the robin type
			SignedMap::iterator it = negativeTypes.upper_bound(value_signed);
			if(it!= negativeTypes.begin()) {
				it--; // need to decrease because upper_bound finds one past the last element of the sequence
				// We found the value in the map, return it
				return static_hcast<RobinType>(it->second);
			} else {
				throw std::logic_error("Cannot find the RobinType for negative long long.");
			}
		} else {
			PyErr_Clear();
			//The number is huge
			return ArgumentPythonLong;
		}
	}
}

Handle<RobinType> BoundedNumericRobinType::giveTypeForNum(PyReferenceBorrow<PyIntObject> num)
{
	static UnsignedMap positiveTypes;
	static SignedMap negativeTypes;
	static bool initialized=false;

	// First make sure that the type maps are initialized
	if(!initialized) {
		initialized = true;

		/*
		 * Some of the types registered here will never
		 * be used in 32 bit platforms but they will not
		 * hurt anyway
		 */

		addPositiveSubtype(positiveTypes,0,15,ArgumentLong);  //type which fits in a short
		addPositiveSubtype(positiveTypes,15,16,ArgumentLong); //type which fits in a unsigned short
		addPositiveSubtype(positiveTypes,16,31,ArgumentLong); //type which fits in an int (an in a long in 32 platforms)
		addPositiveSubtype(positiveTypes,31,32,ArgumentLong); //type which fits in an unsigned int (an in an unsigned long in 32 platforms)
		addPositiveSubtype(positiveTypes,32,63,ArgumentLong); //type which fits in a longlong (an in a long in 64 platforms)
		addPositiveSubtype(positiveTypes,63,64,ArgumentLong); //type which fits in a unsigned longlong (and in a long in 64 platforms)

		addNegativeSubtype(negativeTypes,0,15,ArgumentLong);  //type which fits in a short
		addNegativeSubtype(negativeTypes,15,31,ArgumentLong); //type which fits in an int (an in a long in 32 platforms)
		addNegativeSubtype(negativeTypes,31,63,ArgumentLong); //type which fits in a longlong (an in a long in 64 platforms)

	} //end of initialization

	//Now try to see which kind of number this is
	long value = PyInt_AsLong((PyObject *)num.pointer());
	if(PyErr_Occurred()) {
		PyErr_Clear();
		throw std::logic_error("Cannot obtain a long from a python int.");
	}
	if(value >= 0) {
		//Search for the robin type
		UnsignedMap::iterator it = positiveTypes.lower_bound(value);
		if(it!= positiveTypes.end()) {
			// We found the value in the map, return it
			return static_hcast<RobinType>(it->second);
		} else {
			std::stringstream err;
			err << "Logic-error: Somehow it was not possible to find a C numeric type which fits a negative python int " << value << std::endl;
			throw std::logic_error(err.str().c_str());		}
	} else{
		//the number is a negative value

		//Search for the robin type
		SignedMap::iterator it = negativeTypes.upper_bound(value);
		if(it!= negativeTypes.begin()) {
			it--; // need to decrease because upper_bound finds one past the last element of the sequence
			// We found the value in the map, return it
			return static_hcast<RobinType>(it->second);
		} else {
			std::stringstream err;
			err << "Logic-error: Somehow it was not possible to find a C numeric type which fits a negative python int " << value << std::endl;
			throw std::logic_error(err.str().c_str());
		}
	}
}

BoundedNumericRobinType::~BoundedNumericRobinType()
{

}

std::string BoundedNumericRobinType::getTypeName() const
{
	return m_superType->getTypeName() + " between " + m_min + " and " + m_max;
}

}// end of namespace Robin::Python

}// end of namespace Robin



