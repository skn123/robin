/*
 * stl_python.cc
 *
 *  Created on: Oct 25, 2009
 *      Author: Marcelo Taube
 */
#include <robin/frontends/python/port.h>
#include <Python.h>
#include <vector>
#include "../../../pattern/handle.h"
#include "../../reflection/instance.h"
#include "../../frontends/python/pythonobjects.h"

using namespace Robin;
using namespace Robin::Python;



/**
 * Changes the contents of a vector of longs to reflect the contents of a Python
 * list. Uses the python/C API.
 *
 * If an error occurs the contents of the vector will not be predictable.
 */

template <typename TintType,
		typename TconversionResultType,
		TconversionResultType conversion(PyObject*)>
static PyObject *stl__fillVectorFromList (PyObject *self, PyObject *args)
{
	PyObject *list;
	PyObject *instanceObject;
	if (!PyArg_ParseTuple(args, "OO", &list,&instanceObject))
		return NULL;
	if (!PyList_Check(list)) {
		PyErr_SetString(PyExc_TypeError, "_fillVectorOfLongFromList: the first parameter must be a list");
		return NULL;
	}


	if(!InstanceObject_Check(instanceObject)){
		PyErr_SetString(PyExc_TypeError, "_fillVectorOfLongFromList: the second parameter must be a vector");
		return NULL;
	}

	Handle<Instance> instance = ((InstanceObject*)instanceObject)->getUnderlying();
	// Cannot easily check that instance is a std::vector< TintType > or
	// something typedefed to the same. Should add support for this check in robin.

	std::vector<TintType>*vec = (std::vector<TintType>*)instance->getObject();

	Py_ssize_t length = PyList_Size(list);
	vec->resize(length);
	for (Py_ssize_t i = 0 ; i<length; i++) {
		(*vec)[i] = (TintType)conversion(PyList_GET_ITEM(list, i));
		if(PyErr_Occurred()) {
			return NULL;
		}
	}

	Py_XINCREF(Py_None);
	return Py_None;
}



/**
 * Changes the contents of a Python list to reflect the contents of a vector
 * of longs via Python/C API.
 *
 * If there is a python error the contents of the python list
 * will not be predictable, but still the state of the python interpreter
 * will be valid and garbage collection wont be affected).
 *
 */
template <typename TintType,
		typename TconversionParamType,
		PyObject *(conversion)(TconversionParamType)>
static PyObject *stl__fillListFromVector(PyObject *self, PyObject *args)
{

	PyObject *instanceObject;
	PyObject *list;
	if (!PyArg_ParseTuple(args, "OO", &instanceObject,&list))
	{
		return NULL;
	}

	if(!InstanceObject_Check(instanceObject)){
		PyErr_SetString(PyExc_TypeError, "_fillListFromVectorOfLong: the first parameter must be a vector");
		return NULL;
	}

	if (!PyList_Check(list)) {
		PyErr_SetString(PyExc_TypeError, "_fillListFromVectorOfLong: the second parameter must be a list");
		return NULL;
	}

	Handle<Instance> instance = ((InstanceObject*)instanceObject)->getUnderlying();
	// Cannot easily check that instance is a std::vector< TintType > or
	// something typedefed to the same. Should add support for this check in robin.

	std::vector<TintType>*vec = (std::vector<TintType>*)instance->getObject();

	Py_ssize_t vecLength = vec->size();
	Py_ssize_t listLength = PyList_Size(list);

	// This variable indicates how many elements have to be copied from the vector
	// to the list using PyList_SetItem
	Py_ssize_t needToSet;


	//First check if the two lists are the same
	if(listLength>vecLength) {
		//Removing the extra places in the list
		PyList_SetSlice(list,vecLength,listLength,NULL);
		//vecLength elements have to be copied
		needToSet = vecLength;
	} else if (listLength < vecLength){
		//First setting the newly added places with
		// PyList_Append, later the places which already
		// exist will be set with PyList_SetItem
		for (Py_ssize_t i = listLength; i < vecLength; ++i)
		{
			PyObject *obj = conversion((*vec)[i]);
			if(!obj) {
				return NULL;
			}
			int appendReturnCode = PyList_Append(list, obj );
			if(appendReturnCode != 0) {
				return NULL;
			}
		}

		//listLength elements have to be setted, the rest were already appended
		needToSet = vecLength;
	} else {
		//The vector and the list are the same size, need to copy all the elements
		needToSet = vecLength;
	}
	//now the list and the vector have the same length
	// (the following line does nothing, its for decumentation). :)
	listLength = vecLength;

	for (Py_ssize_t i = 0; i < needToSet; ++i)
	{
		PyObject *obj = conversion((*vec)[i]);
		if(!obj) {
			return NULL;
		}
		int rc = PyList_SetItem(list,i,obj);
		if(rc != 0) {
			return NULL;
		}
	}
	Py_XINCREF(Py_None);
	return Py_None;
}

inline unsigned long long pythonNumber_to_ulonglong(PyObject *obj) {
	if(PyInt_Check(obj)) {
		//long fits in an ulonglong
		long a = PyInt_AS_LONG(obj);
		if(a<0) {
			PyErr_SetString(PyExc_TypeError,"cannot convert negative integer to ulonglong");
			return 0;
		} else {
			return a;
		}
	} else {
		//will fail if type is not long, but i think is ok
		return PyLong_AsUnsignedLongLong(obj);
	}
}

static PyMethodDef methods[] = {
    { "_fillVectorOfLongFromList",&stl__fillVectorFromList<long,long,PyInt_AsLong>, METH_VARARGS,"It recieves a 'list' of ints and a 'std::vector<long>' and fills the second one with the contents of the first(any previous content is removed)."},
    { "_fillListFromVectorOfLong",&stl__fillListFromVector<long,long,PyInt_FromLong>, METH_VARARGS,"It recieves a 'std::vector<long>' and a 'list' of ints and fills the second one with the contents of the first(any previous content is removed)."},
    { "_fillVectorOfIntFromList",&stl__fillVectorFromList<int,long,PyInt_AsLong>, METH_VARARGS,"It recieves a 'list' of ints and a 'std::vector<int>' and fills the second one with the contents of the first(any previous content is removed)."},
    { "_fillListFromVectorOfInt",&stl__fillListFromVector<int,long,PyInt_FromLong>, METH_VARARGS,"It recieves a 'std::vector<unsigned int>' and a 'list' of ints and fills the second one with the contents of the first(any previous content is removed)."},
    { "_fillVectorOfUnsignedIntFromList",&stl__fillVectorFromList<unsigned int,long,PyInt_AsLong>, METH_VARARGS,"It recieves a 'list' of ints and a 'std::vector<unsigned int>' and fills the second one with the contents of the first(any previous content is removed)."},
    { "_fillListFromVectorOfUnsignedInt",&stl__fillListFromVector<unsigned int,long,PyInt_FromLong>, METH_VARARGS,"It recieves a 'std::vector<int>' and a 'list' of ints and fills the second one with the contents of the first(any previous content is removed)."},
    { "_fillVectorOfUnsignedLongLongFromList",&stl__fillVectorFromList<unsigned long long, unsigned long long, pythonNumber_to_ulonglong>, METH_VARARGS,"It recieves a 'list' of longs and a 'std::vector<unsigned long long>' and fills the second one with the contents of the first(any previous content is removed)."},
    { "_fillListFromVectorOfUnsinedLongLong",&stl__fillListFromVector<unsigned long long, unsigned long long, PyLong_FromUnsignedLongLong>, METH_VARARGS,"It recieves a 'std::vector<unsigned long long>' and a 'list' of longs and fills the second one with the contents of the first(any previous content is removed)."},
    { "_fillVectorOfLongLongFromList",&stl__fillVectorFromList<long long, long long, PyLong_AsLongLong>, METH_VARARGS,"It recieves a 'list' of ints and a 'std::vector<long long>' and fills the second one with the contents of the first(any previous content is removed)."},
    { "_fillListFromVectorOfLongLong",&stl__fillListFromVector<long long, long long, PyLong_FromLongLong>, METH_VARARGS,"It recieves a 'std::vector<long long>' and a 'list' of ints and fills the second one with the contents of the first(any previous content is removed)."},
    { 0 }
};


#ifndef _ROBIN_SPEC
#error The macro _ROBIN_SPEC has to be defined in the compiler command line
#error It defines the suffix added to all the library names to specify the current version
#endif

//ROBININIT generates the initializing function of the module
#define _ROBININIT(NAME,SPEC) init##NAME##SPEC
#define ROBININIT(NAME,SPEC) _ROBININIT(NAME,SPEC)

//ROBINMODULESTRING generates the name of the module quoted
#define _ROBINSTRING(PARAM) #PARAM
#define ROBINSTRING(PARAM) _ROBINSTRING(PARAM)
#define ROBINMODULESTRING(NAME,SPEC) ROBINSTRING(NAME) ROBINSTRING(SPEC)


void
initlibstl_py();

//initializing function
PyMODINIT_FUNC
ROBININIT(libstl_py,_ROBIN_SPEC) () {
	initlibstl_py();
};

void
initlibstl_py()
{
	// Register Python module
	Py_InitModule4(ROBINMODULESTRING(libstl_py,_ROBIN_SPEC), methods,
			   "STL conversions optimization for python", NULL, PYTHON_API_VERSION);
}

