/*
 * port.h
 *
 *  Created on: Mar 31, 2010
 *      Author: Marcelo Taube
 *
 *  This file is used to define variables which are part of some
 *  versions of python but not all, thus making it easier to write
 *  version compatible code.
 */

#ifndef ROBIN_FRONTENDS_PYTHON_PORT_H_
#define ROBIN_FRONTENDS_PYTHON_PORT_H_

#include <Python.h>


/*
 * Defining Py_ssize_t for old versions of python
 */
#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MAX)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
inline static Py_ssize_t PyInt_AsSsize_t(PyObject *obj) {
	return PyInt_AsLong(obj);
}
#endif


#endif /* ROBIN_FRONTENDS_PYTHON_PORT_H_ */
