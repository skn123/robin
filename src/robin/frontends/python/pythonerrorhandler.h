#ifndef ROBIN_FRONTENDS_PYTHON_ERROR_HANDLER
#define ROBIN_FRONTENDS_PYTHON_ERROR_HANDLER

#include "../../reflection/error_handler.h"

namespace Robin {

namespace Python {

class PythonErrorHandler : public ErrorHandler
{
public:
	virtual ~PythonErrorHandler() { }

	virtual void setError(scripting_element error);

	virtual scripting_element getError();

private:
	/** The error that has been set */
	scripting_element m_error;
};

}

}

#endif
