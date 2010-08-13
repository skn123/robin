/*
 * parameters.h
 *
 *  Created on: Oct 26, 2009
 *      Author: Marcelo Taube
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include <string>
#include <map>
#include <set>
#include <vector>

namespace Robin {


typedef void *scripting_element;
typedef std::vector<scripting_element> ActualArgumentList;
typedef scripting_element ActualArgumentArray[];
typedef std::map<std::string, scripting_element> KeywordArgumentMap;
typedef std::set<std::string> ArgumentsSet;


/**
 * Base class for all the exceptions related to a wrong call.
 * Including wrong arguments or an unexistant method.
 */
class CannotCallException : public std::exception
{
public:
		virtual ~CannotCallException() throw() = 0;
};

inline CannotCallException::~CannotCallException() throw()
{

}

};


#endif /* PARAMETERS_H_ */
