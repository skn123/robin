
#include "companion.h"



namespace {
	class Initializer {
	public:	Initializer() { import_array(); }
	} _;
}


long ManyTimes::sum_1D(PyArrayObject *array)
{
	PyObject *arrayobj = (PyObject*)array;
	char *data;
	int len;

	if (PyArray_As1D(&arrayobj, &data, &len, PyArray_INT) == 0) {
		long sum = 0;
		for (int i = 0; i < len; ++i)
			sum += ((int*)data)[i];
		return sum;
	}
	else {
		fprintf(stderr, "// OUCH! Numeric array error.\n");
		return -1;
	}
}


PyArrayObject *ManyTimes::new_1D(int length)
{
	int dims[] = { length };
	return (PyArrayObject*)PyArray_FromDims(1, dims, PyArray_INT); 
}
