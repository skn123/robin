
#include "../../frontends/python/facade.h"
#include "language.h"
#include <math.h>



PyObject *py_typemap(PyObject *self, PyObject *args)
{
	const char *typen;

	if (PyArg_ParseTuple(args, "s:typemap", &typen)) {
		return (PyObject*)Robin::Python::Facade::type(typen);
	}
	else
		return NULL;
}


PyObject *py_dataroot(PyObject *self, PyObject *args)
{
	int value;

	if (PyArg_ParseTuple(args, "i:dataroot", &value)) {
		DataMembers *dm = new DataMembers(0);
		dm->square = value;
		return Robin::Python::Facade::fromObject("DataMembers", dm);
	}
	else
		return NULL;
}

PyObject *py_datafactor(PyObject *self, PyObject *args)
{
	PyObject *pydata;

	if (PyArg_ParseTuple(args, "O:datafactor", &pydata)) {
		DataMembers *dm = Robin::Python::Facade
			::asObject<DataMembers>("DataMembers", pydata);
		if (!dm) return NULL;

		return PyFloat_FromDouble(sqrt(dm->square));
	}
	else
		return NULL;
}


extern "C"
void initfacade()
{
	static PyMethodDef methods[] = {
		{ "typemap", &py_typemap,     METH_VARARGS },
		{ "root",    &py_dataroot,    METH_VARARGS },
		{ "factor",  &py_datafactor,  METH_VARARGS },
		{ 0 }
	};

	// Register Python module
	PyObject *module =
		Py_InitModule4("facade", methods,
					   "Robin's Facade Testing Module",
					   NULL, PYTHON_API_VERSION);

}
