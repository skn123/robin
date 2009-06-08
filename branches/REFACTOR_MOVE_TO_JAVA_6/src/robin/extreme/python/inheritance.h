// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#ifndef ROBIN_EXTREME_TEST_INHERITANCE
#define ROBIN_EXTREME_TEST_INHERITANCE

#include <vector>
#include <string>


namespace B {

/**
 * An abstract class that should be implemented in Python.
 */
class Functor
{
public:
	virtual std::string operate(const std::string& arg0, int arg1) const = 0;
    virtual float factor() const { return 1.0; }
};

}

/**
 * An implementation in C++ which can also be overridden in Python.
 */
class FunctorImpl : public B::Functor
{
public:
    virtual std::string operate(const std::string& arg0, int arg1) const
    { return arg0; }

protected:
	double z;
};

std::vector<std::string> mapper(std::vector<std::string> strings,
								B::Functor *functor)
{
	std::vector<std::string> results;
	for (size_t i = 0; i < strings.size(); ++i) {
		results.push_back(functor->operate(strings[i], i));
	}
	return results;
}							

float mul(float num, B::Functor *functor)
{
    return num * functor->factor();
}

/*
 TODO: I want this to work, but there is a bug in wrapping std::vector<T*>
float reducer(std::vector<Functor*> functors)
{
    float prod = 1.0;
    for (size_t i = 0; i < functors.size(); ++i) {
        prod *= functors[i]->factor();
    }
    return prod;
}
*/

class TaintedVirtual
{
    public:
        TaintedVirtual(int taintValue)
            : m_taintValue(taintValue)
        {}

        virtual ~TaintedVirtual() {}

        virtual int returnTaint() {
            return m_taintValue;
        }

        virtual int returnFilth() {
            return -1; // infinitely filthy
        }

    private:
        int m_taintValue;
};



#endif
