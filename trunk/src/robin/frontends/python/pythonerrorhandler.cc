#include "pythonerrorhandler.h"

namespace Robin {

namespace Python {

void PythonErrorHandler::setError(scripting_element error)
{
	m_error = error;
}

scripting_element PythonErrorHandler::getError()
{
	return m_error;
}

}

}
