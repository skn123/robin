// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par PACKAGE
 * Robin
 */

#include "facade.h"


namespace Robin {

namespace Python {


/**
 * Returns the Robin ClassObject associated with this converter.
 */
template < class T >
PyTypeObject *xParamConverter<T>::PythonType() const
{
	return Facade::type(m_robin_classname.c_str());
}

/**
 * Returns the C type_info associated with this converter.
 */
template < class T >
const std::type_info *xParamConverter<T>::CType() const
{
	return &typeid(T);
}

/**
 * Creates a wrapped instance by consuming the value held in a ParamValue.
 */
template < class T >
PyObjectHandle xParamConverter<T>::ParamValue2PyObject(xParam::ParamValue& 
													   xparamvalue)
{
	// Get C++ object
	Handle<T> cppo = xParam_internal::extract(xparamvalue, 
											  xParam::TypeTag<T>());
	// Create a PyObject (exploit T's copy constructor)
	return PyObjectHandle(Facade::fromObject(m_robin_classname.c_str(),
											 new T(*cppo)));
}

/**
 * Creates a ParamValue object by extracting the value of a wrapped PyObject.
 */
template < class T >
Handle<xParam::ParamValue> xParamConverter<T>::PyObject2ParamValue(PyObject *
															   pythonobject)
{
	// Get C++ object
	T *cppo;
	cppo = Facade::asObject<T>(m_robin_classname.c_str(), pythonobject);
	// Create a ParamValue
	return xParam_internal::make_value_copy_ptr(cppo);
}


/**
 * A convenience function for associating xParam types and Robin types.
 *
 * @param robin_classname the name of the class as Robin sees it (fully
 * qualified name, without any typedefs).
 */
template < class T >
void registerxParamType(const char *robin_classname)
{
	xParam::registerPythonType(
		xParam_internal::Handle<xParam::PythonConverter>(
		    new Robin::Python::xParamConverter<T>(robin_classname, 0,0)
		)
	);
}

} // end of namespace Robin::Python

} // end of namespace Robin
