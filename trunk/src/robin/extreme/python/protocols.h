// -*- c++ -*-

#ifndef ROBIN_TEST_PYTHON_PROTOCOLS_H
#define ROBIN_TEST_PYTHON_PROTOCOLS_H

class Times
{
public:
	Times(int factor) : x_factor(factor) { }

	int mul(int units) const { return x_factor * units; }
	void triangle(int units, int product) { x_factor = product / units; }
	void omit(int units) { x_factor -= units; }


	Times operator+(const Times& other) const 
		{ return Times(x_factor + other.x_factor); }
	Times operator-(const Times& other) const 
		{ return Times(x_factor - other.x_factor); }
	Times combine(const Times& other) const 
		{ return Times(x_factor * other.x_factor); }
	Times neg_combine(const Times& other) const
		{ return Times(-x_factor * other.x_factor); }

	const char cid() const { return 'T'; }
	const char *classid() const { return "Times"; }

	operator int() const { return x_factor; }

	const int& factoref() const { return x_factor; }
	
	bool operator==(const Times& other) { return x_factor == other.x_factor; }
	bool operator!=(const Times& other) { return x_factor != other.x_factor; }
	bool operator<(const Times& other) { return x_factor < other.x_factor; }
	bool operator>(const Times& other) { return x_factor > other.x_factor; }

	friend bool operator^(const Times& first, const Times& second);

private:
	int x_factor;
};
bool operator^(const Times& first, const Times& second);


inline Times operator%(const Times& t1, int value) { return t1; }

#endif
