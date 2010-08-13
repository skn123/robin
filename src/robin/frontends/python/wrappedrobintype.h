/*
 * wrappedrobintype.h
 *
 *  Created on: Jan 19, 2010
 *      Author: Marcelo Taube
 */

#include "robinpyobject.h"
#include "pyhandle.h"
#include <pattern/handle.h>
#include <robin/reflection/robintype.h>
namespace Robin {

namespace Python {


/**
 * This object represents a RobinType so it can be accessed from
 * the python shell.
 */
class WrappedRobinType : public RobinPyObject
{
private:
	WrappedRobinType(Handle<RobinType> underlying);

public:
	/**
	 * Used as a constructor
	 */
	static PyReferenceSteal<WrappedRobinType> construct(Handle<RobinType> underlying);
	static PyObject *pythonConstruct(PyTypeObject *self, PyObject *args, PyObject *kwargs );

	virtual ~WrappedRobinType();
	PyObject *__repr__();
	PyObject *__getattr__(const char *name);

	static PyObject *__repr__(PyObject *self);
	static PyObject *__getattr__(PyObject *self, char *name);

	Handle<RobinType> m_underlying;
};

extern PyTypeObject WrappedRobinTypeTypeObject;


} //end of namespace Python::Robin

} //end of namespace Python
