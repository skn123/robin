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

#include <map>
#include <list>
#include <robin/frontends/frontend.h>


extern "C"
#ifdef _WIN32
__declspec(dllexport)
#endif
void initrobin();

struct _dictobject;
struct _typeobject;


namespace Robin {

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
	~PythonFrontend();

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
	virtual 
	Handle<TypeOfArgument> detectType(scripting_element element) const;
	virtual Insight detectInsight(scripting_element element) const;

	Handle<TypeOfArgument> detectType(struct _typeobject *pytype) const;
	//@}

	/**
	 * @name Active Translation
	 */

	//@{
	virtual
	Handle<Adapter> giveAdapterFor(const TypeOfArgument& type) const;

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
	ClassObject    *getClassObject(Handle<Robin::Class> clas) const;
	ClassObject    *getClassObject(const std::string& name) const;
	EnumeratedTypeObject    
	               *getEnumeratedTypeObject(Handle<EnumeratedType> enumtype) 
		const;
	FunctionObject *getFunctionObject(const std::string& name) const;
	TemplateObject *getTemplateObject(const std::string& name) const;
	void            setTemplateObject(const std::string& name,
									  TemplateObject *pytemplate);
	void            addUserDefinedType(Handle<UserDefinedTranslator> 
									   translator);
	//@}

protected:
	enum TemplateKind { T_CLASS, T_FUNCTION };

	bool getTemplateName(const std::string& classname, 
	                     std::string& templatename,
	                     std::vector<std::string>& templateargs);
	TemplateObject *exposeTemplate(TemplateKind kind,
								   const std::string& classname,
								   std::string& templatename);
	void *getRegisteredObject(TemplateKind kind, const std::string& name);

	typedef std::map<const Robin::Class*, Robin::Python::ClassObject*> 
	    classassoc;
	typedef std::map<std::string, Robin::Python::ClassObject*> 
	    classnameassoc;
	typedef std::map<const Robin::EnumeratedType*, 
					 Robin::Python::EnumeratedTypeObject*> 
	    enumassoc;
	typedef std::map<std::string, Robin::Python::FunctionObject*>
		funcnameassoc;
	typedef std::map<std::string, Robin::Python::TemplateObject*> 
	    templatenameassoc;
	typedef std::list<Handle<UserDefinedTranslator> >
		userdefinedtypelist;

	mutable classassoc        m_classes;
	mutable classnameassoc    m_classesByName;
	mutable enumassoc         m_enums;
	mutable funcnameassoc     m_functionsByName;
	templatenameassoc         m_templatesByName;
	userdefinedtypelist       m_userTypes;

	class Module;
	friend void ::initrobin();

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
	ByTypeTranslator(struct _typeobject *pytype);

	virtual Handle<TypeOfArgument> detectType(scripting_element element);

private:
	Handle<TypeOfArgument> m_type;
	struct _typeobject *m_pytype;
};


} // end of namespace Robin::Python

} // end of namespace Robin

#endif

