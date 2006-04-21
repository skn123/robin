// -*- c++ -*-

#ifndef ROBIN_TEST_PYTHON_LANGUAGE_FEATURES_H
#define ROBIN_TEST_PYTHON_LANGUAGE_FEATURES_H

#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>


// ----------------------------------------------------------------------

class El { };

enum Em { EM };

class DataMembers
{
public:
	DataMembers(long factor) : square(factor * factor), m(EM) { }

	int square;
	El e;
	Em m;

	static const int zero;

private:
	int hidden;
};

const int DataMembers::zero = 0;


class PrimitiveTypedef
{
public:
	typedef unsigned int factor_t;

	PrimitiveTypedef(factor_t factor) : m_factor(factor) { }

private:
	factor_t m_factor;
};

// ----------------------------------------------------------------------

class EnumeratedValues
{
public:
	enum PublicEnum { PB_FIRST, PB_SECOND, PB_THIRD };

	PublicEnum pbm;

private:
	enum PrivateEnum { PV_FIRST, PV_SECOND };

	PrivateEnum pvm;
};

struct Structs
{
public:
	struct PublicStruct {
		char member;
	};

	PublicStruct pbs;

private:
	struct PrivateStruct {
		char member;
	};
	PrivateStruct pvs;
};

class Aliases
{
};

typedef Aliases Aliased;

class DerivedFromAlias : public Aliased
{
};

std::ostream& operator<<(std::ostream& o, const Aliased& a)
{
	return o << "Aliased";
}

// ----------------------------------------------------------------------

class Inners
{
public:
	struct StructIn
	{
		struct StructInStruct { };
	};

	static void staticMethodIn() { }
};

// ----------------------------------------------------------------------

class Constructors
{
public:
	explicit Constructors(Aliased alias) { }

private:
	Constructors() { }
};

// ----------------------------------------------------------------------

/**
 * @par .robin
 * assignment, cloneable
 */
class AssignmentOperator
{
public:
	AssignmentOperator(float factor) : x_factor(factor) { }

	void report() const 
	{ fprintf(stderr, "AssignmentOperator: %f\n", x_factor); }

	float x_factor;
};

// ----------------------------------------------------------------------

class Conversions
{
public:
	struct Edge { int i; int j; };

	void apply(Edge ft) { }
};

// ----------------------------------------------------------------------

class Exceptions
{
public:
	void cry() { throw El(); }
};

// ----------------------------------------------------------------------

class Interface
{
public:
	virtual const void *abstraction() const = 0;
};

class Abstract : public Interface
{
public:
	/* does not implement abstraction() */
	void util() const { }
};

class NonAbstract : public Abstract
{
public:
	virtual const void *abstraction() const { return 0; }

	static Interface *factorize() { return new NonAbstract; }
};

// ----------------------------------------------------------------------

class Primitives
{
public:

	double getADouble()
	{
		m_double = getRandFloat();
		return m_double;
	}

	const double& getADoubleRef()
	{
		return m_double;
	}

	std::string getAStringDouble()
	{
		return double2string(m_double);
	}

	float getAFloat()
	{
		m_float = getRandFloat();
		return m_float;
	}

	std::string getAStringFloat()
	{
		return float2string(m_float);
	}

	std::vector<float> getManyFloats()
	{
		m_floats.resize(0);
		for (int i = 0; i < 20; ++i) {
			m_floats.push_back(getRandFloat());
		}
		return m_floats;
	}

	std::vector<std::string> getManyStringFloats()
	{
		std::vector<std::string> sfloats;
		for (int i = 0; i < m_floats.size(); ++i) {
			sfloats.push_back(float2string(m_floats[i]));
		}
		return sfloats;
	}

	void shorten(short s) { }
	void ushorten(unsigned short s) { }

	void unsupported(const char val[]) { }

private:

	float getRandFloat()
	{
		return float(rand()) / 5000.0f;
	}

	std::string float2string(float val)
	{
		char sfloat[30];
		sprintf(sfloat, "%.3f", val);
		return std::string(sfloat);
	}

	std::string double2string(double val)
	{
		char sdouble[30];
		sprintf(sdouble, "%.3lf", val);
		return std::string(sdouble);
	}
	
	float m_float;
	double m_double;
	std::vector<float> m_floats;
};

// ----------------------------------------------------------------------

// - preprocessor challenge

#include "stls.h"

#ifdef __doxygen
#include "does_not_exist.h"
#endif


#endif