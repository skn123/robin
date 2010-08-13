// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par PACKAGE
 * Robin
 *
 * Integration of Robin and xParam for Python.
 */

#ifndef ROBIN_FRONTENDS_PYTHON_XPARAM_H
#define ROBIN_FRONTENDS_PYTHON_XPARAM_H


// xParam includes
#include <xparam_py_registry.h>
#include <xparam_py_conversion_api.h>


namespace Robin {

namespace Python {


using xParam_internal::Handle;


template < class T >
class xParamConverter : public xParam::PythonConverterPartialImp
{
public:
	xParamConverter(const std::string& robin_classname,
					xParam::priority_t py_priority,
					xParam::priority_t xp_priority)
		: xParam::PythonConverterPartialImp(py_priority, xp_priority),
		  m_robin_classname(robin_classname) { }

	/**
     * @name Properties
	 */
	//@{
	PyTypeObject *PythonType() const;
	const std::type_info *CType() const;
	//@}

	/**
	 * @name Conversion
	 */
	//@{
	PyObjectHandle ParamValue2PyObject(xParam::ParamValue& xparamvalue);

	Handle<xParam::ParamValue> PyObject2ParamValue(PyObject *pythonobject);
	//@}

protected:
	const std::string m_robin_classname;
};


template < class T >
void registerxParamType(const char *robin_classname);

/**
 * A shortcut for registerxParamType when the robin_classname and the
 * name of the type in C are identical (a common situation).
 */
#define ROBIN_PYTHON_XPARAM(CLASS) \
	Robin::Python::registerxParamType<CLASS>(#CLASS)


} // end of namespace Robin::Python

} // end of namespace Robin


#include "python_xparam.inl"


#endif
