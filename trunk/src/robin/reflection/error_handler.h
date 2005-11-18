#ifndef ROBIN_REFLECTION_ERROR_HANDLER
#define ROBIN_REFLECTION_ERROR_HANDLER

// Robin includes
#include "typeofargument.h"

namespace Robin {

/**
 * This interface allows setting and retrieving errors, both in the scripting
 * language and from user wrapped code.
 * It is implemented in the frontends.
 */
class ErrorHandler
{
public:
	virtual ~ErrorHandler() { }
	
	/**
	 * Sets the error to be the given scripting environment error.
	 *
	 * @param error the information of the error that has occured
	 */
	virtual void setError(scripting_element error) = 0;

	/**
	 * Retrieves the error set in the scripting environment.
	 * This need not be the same error that has been set using setError.
	 * It can be a processed version of that error, or a whole other error
	 * altogether. It need not even follow a call to setError.
	 *
	 * @return the information of the error that has occured
	 */
	virtual scripting_element getError() = 0;
};

}

#endif
