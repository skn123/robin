// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/python/pythonfrontend.h
 *
 * @par TITLE
 * Python Frontend Implementation
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_PYTHON_FRONTEND_H
#define ROBIN_PYTHON_FRONTEND_H
#include "port.h"
#include <map>
#include <list>
#include <robin/frontends/frontend.h>
#include "pyhandle.h"

extern "C"
#ifdef _WIN32
__declspec(dllexport)
#endif
void initlibrobin_pyfe();

struct _dictobject;
struct _typeobject;


namespace Robin {

class Namespace;
class Class;
class EnumeratedType;

/**
 * @brief A complete front-end for the Python scripting environment.
 */
namespace Python { 

class ClassObject;
class EnumeratedTypeObject;
class FunctionObject;
class Facade;

typedef _dictobject TemplateObject;

/**
 * @class PythonFrontend
 * @nosubgrouping
 *
 * Implements a frontend for the Python interpreter 
 * using the native Python API.
 */
class PythonFrontend : public Frontend
{
public:
	PythonFrontend();
	virtual ~PythonFrontend();

	/**
	 * @name Deployment
	 */

	//@{
	virtual
	void initialize() const;

	//@}

	/**
	 * @name Passive Translation
	 */

	//@{

protected:

	/**
	 * This protected method is used to detect the RobinType when it
	 * is unambiguous. Is used by both detectType_mostSpecific and detectType_asPython.
	 */
	inline virtual Handle<RobinType> detectType_basic(PyObject *element) const;

public:

	/**
	 * It computes the RobinType of an object, if the object belongs
	 * to several types it will report the most specific of those types.
	 *  (a type which is a subtype of all the other types of the object).
	 */
	virtual Handle<RobinType> detectType_mostSpecific(scripting_element element) const;

	/**
	 * It computes the RobinType of an object. If the object belongs
	 * to several types it will report the type imitating the behavior
	 * of the 'type' function in python.
	 * For example type([3]) == list, so it detectType_asPython will return
	 * a general ListRobinType. No information will be given about
	 * the type of the elements inside.
	 */
	virtual Handle<RobinType> detectType_asPython(PyObject *element) const;


	Handle<RobinType> detectType(struct _typeobject *pytype) const;
	//@}

	/**
	 * @name Active Translation
	 */

	//@{
	virtual
	Handle<Adapter> giveAdapterFor(const RobinType& type) const;

	//@}

	/**
	 * @name Exposure
	 */

	//@{
	virtual
	void exposeLibrary(const Library& newcomer);
	//@}

	/**
	 * @name Memory Management
	 */

	//@{
	virtual
	scripting_element duplicateReference(scripting_element element);

	virtual
	void release(scripting_element element);

    virtual
    void own(scripting_element master, scripting_element slave);

    virtual
    void bond(scripting_element master, scripting_element slave);
	//@}
	
	/**
	 * @name Low Level
	 */

	//@{
	virtual
	const LowLevel& getLowLevel() const;

	virtual
	const Interceptor& getInterceptor() const;

	virtual
	ErrorHandler& getErrorHandler();
	//@}

	/**
	 * @name Auxiliary
	 */
	//@{
	PyTypeObject   *getTypeObject(const std::string& name) const;
	ClassObject    *getClassObject(Handle<Robin::Class> clas) const;
	ClassObject    *getClassObject(const std::string& name) const;
	EnumeratedTypeObject    
	               *getEnumeratedTypeObject(Handle<EnumeratedType> enumtype) 
		const;
	PyReferenceBorrow<FunctionObject> getFunctionObject(const std::string& name) const;
	TemplateObject *getTemplateObject(const std::string& name) const;
	void            setTemplateObject(const std::string& name,
									  TemplateObject *pytemplate);
	void            addUserDefinedType(Handle<UserDefinedTranslator> 
									   translator);
	//@}

protected:
	enum TemplateKind { T_CLASS, T_FUNCTION };
	static bool getTemplateName(const std::string& classname,
	                     std::string& templatename,
	                     std::vector<std::string>& templateargs);
	TemplateObject *exposeTemplate(Handle<Namespace>& namespc, TemplateKind kind,
								   const std::string& classname,
								   std::string& templatename);
	PyObject *getRegisteredObject(TemplateKind kind, const std::string& name);

	typedef std::map<std::string, PyTypeObject*> typenameassoc;
	typedef std::map<const Robin::Class*, Robin::Python::ClassObject * >
	    classassoc;
	typedef std::map<std::string, Robin::Python::ClassObject*> 
	    classnameassoc;
	typedef std::map<const Robin::EnumeratedType*,
					 Robin::Python::EnumeratedTypeObject*> 
	    enumassoc;
	typedef std::map<std::string, PyReferenceCreate<Robin::Python::FunctionObject> >
		funcnameassoc;
	typedef std::map<std::string, Robin::Python::TemplateObject*> 
	    templatenameassoc;
	typedef std::list<Handle<UserDefinedTranslator> >
		userdefinedtypelist;

	typenameassoc             m_typesByName;
	mutable classassoc        m_classes;
	mutable classnameassoc    m_classesByName;
	mutable enumassoc         m_enums;
	mutable funcnameassoc     m_functionsByName;
	templatenameassoc         m_templatesByName;
	userdefinedtypelist       m_userTypes;

	class Module;
	friend void ::initlibrobin_pyfe();

	friend class Robin::Python::Facade;

	LowLevel     *m_lowLevel;
	Interceptor  *m_interceptor;
	ErrorHandler *m_errorHandler;

public:
	static const std::string DATAMEMBER_PREFIX;
	static const std::string SINKMEMBER_PREFIX;
};


class ByTypeTranslator : public UserDefinedTranslator
{
public:
	/*
	 * @param pytype The type object. Regarding memory management: pytpe is passed
	 * as a borrowed reference.
	 */
	ByTypeTranslator(struct _typeobject *pytype);
	virtual ~ByTypeTranslator();
	virtual Handle<RobinType> detectType(scripting_element element);

private:
	Handle<RobinType> m_type;

	/*
	 *  A weak reference to the type of this translator
	 */
	struct _typeobject *m_pytype;
};


} // end of namespace Robin::Python

} // end of namespace Robin

#endif

