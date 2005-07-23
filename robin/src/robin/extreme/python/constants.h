#ifndef ROBIN_PYTHON_CHECK_CONSTANTS_H
#define ROBIN_PYTHON_CHECK_CONSTANTS_H

#include <iostream>

class MathConstant
{
public:
	MathConstant(const char *name, double value)
		: m_name(name), m_value(value) { }

private:
	const char *m_name;
	const double m_value;

	friend std::ostream& operator <<(std::ostream& os, const MathConstant& mc);
};

std::ostream& operator <<(std::ostream& os, const MathConstant& mc)
{
	os << mc.m_name << '=' << mc.m_value;
	return os;
}


//@{
/**
 * @par .robin
 * available
 */
extern const MathConstant pi;
extern const MathConstant e;
//@}

#ifndef __doxygen
const MathConstant pi("pi", 3.1415);
const MathConstant e("e", 2.37);
#endif

#endif
