// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#ifndef ROBIN_EXTREME_TEST_INHERITANCE
#define ROBIN_EXTREME_TEST_INHERITANCE

#include <vector>
#include <string>


/**
 * An abstract class that should be implemented in Python.
 */
class Functor
{
public:
	virtual std::string operate(const std::string& arg0, int arg1) const = 0;
};


std::vector<std::string> mapper(std::vector<std::string> strings,
								Functor *functor)
{
	std::vector<std::string> results;
	for (size_t i = 0; i < strings.size(); ++i) {
		results.push_back(functor->operate(strings[i], i));
	}
	return results;
}							


#endif
