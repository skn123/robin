#include <string>
/*
 * pythonerror.h
 *
 *  Created on: Jan 27, 2010
 *      Author: Marcelo Taube
 */

namespace Robin {

namespace Python {

/**
 * It reports an error which happened while running a python callable.
 * It throws an std::runtime_error with all the contents of the error
 *
 * @param exceptionType A string name for the exception which will be
 * 					printed to the user, example: 'RealBadProblemException'
 * @param doing What was the program doing when the exception ocurred, like
 * 				"dividing x by y".
 */
void reportPythonError(std::string exceptionType, std::string doing);

/**
 * Like reportPythonError but in string format instead of rising an exception.
 * Notice that if this fails to get the proper error message it might still
 * rise an exception.
 */
std::string getPythonErrorAsString(std::string exceptionType, std::string doing);


}
}
