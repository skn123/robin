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


namespace Robin {

class Class;

/**
 * @brief A complete front-end for the Python scripting environment.
 */
namespace Python { 

class ClassObject;
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
	//@}
	
	/**
	 * @name Low Level
	 */

	//@{
	virtual
	const LowLevel& getLowLevel() const;

	virtual
	const Interceptor& getInterceptor() const;
	//@}

	/**
	 * @name Auxiliary
	 */
	//@{
	ClassObject    *getClassObject(Handle<Robin::Class> clas) const;
	ClassObject    *getClassObject(const std::string& name) const;
	TemplateObject *getTemplateObject(const std::string& name) const;
	void            setTemplateObject(const std::string& name,
									  TemplateObject *pytemplate);
	void            addUserDefinedType(Handle<UserDefinedTranslator> 
									   translator);
	//@}

protected:
	bool getTemplateName(const std::string& classname, 
	                     std::string& templatename,
	                     std::vector<std::string>& templateargs);
	TemplateObject *exposeTemplate(const std::string& classname,
								   std::string& templatename);

	typedef std::map<const Robin::Class*, Robin::Python::ClassObject*> 
	    classassoc;
	typedef std::map<std::string, Robin::Python::ClassObject*> 
	    classnameassoc;
	typedef std::map<std::string, Robin::Python::TemplateObject*> 
	    templatenameassoc;
	typedef std::list<Handle<UserDefinedTranslator> >
		userdefinedtypelist;

	mutable classassoc        m_classes;
	mutable classnameassoc    m_classesByName;
	templatenameassoc         m_templatesByName;
	userdefinedtypelist       m_userTypes;

	class Module;
	friend void ::initrobin();

	friend class Robin::Python::Facade;

	LowLevel    *m_lowLevel;
	Interceptor *m_interceptor;

public:
	static const std::string DATAMEMBER_PREFIX;
};

} // end of namespace Robin::Python

} // end of namespace Robin

#endif

