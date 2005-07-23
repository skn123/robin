
#ifndef ROBIN_TEST_PYTHON_NUMERICAL_H
#define ROBIN_TEST_PYTHON_NUMERICAL_H

#define PyArray_API PyArray_API_companion

#include <Python.h>
#include <Numeric/arrayobject.h>



/**
 * Checks interaction between Robin and Numerical Python.
 */
class ManyTimes
{
public:
	long sum_1D(PyArrayObject *array);
	PyArrayObject *new_1D(int length);
};



#endif
