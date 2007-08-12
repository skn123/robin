// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/python/pythonfrontend.cc
 *
 * @par TITLE
 * Python Frontend Implementation
 *
 * @par PACKAGE
 * Robin
 */

#include <Python.h>
#include "pythonfrontend.h"

#include <assert.h>

// Robin includes
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/namespace.h>
#include <robin/reflection/library.h>
#include <robin/reflection/class.h>
#include <robin/reflection/address.h>
#include <robin/reflection/enumeratedtype.h>
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/instance.h>
#include <robin/reflection/fundamental_conversions.h>

// Python Frontend includes
#include "pythonadapters.h"
#include "pythonobjects.h"
#include "pythonconversions.h"

// Python Low Level includes
#include "pythonlowlevel.h"
#include "pythoninterceptor.h"
#include "pythonerrorhandler.h"


namespace Robin {

namespace Python {


const std::string PythonFrontend::DATAMEMBER_PREFIX = ".data_";
const std::string PythonFrontend::SINKMEMBER_PREFIX = ".sink_";
extern PyObject *pydouble;
extern PyObject *pychar;
extern PyObject *pylong_long;
extern PyObject *pyunsigned_long;
extern PyObject *pyunsigned_int;
extern PyObject *pyunsigned_long_long;
extern PyObject *pyunsigned_char;
extern PyObject *pysigned_char;


namespace {

	PyObject *pyowned(PyObject *object)
	{
		Py_XINCREF(object);
		return object;
	}

	PyObject *splitName(const std::string& fullname)
	{
		PyObject *pylist = PyList_New(0);

		unsigned int bracket_balance = 0;
		std::string::const_iterator start = fullname.begin();

		for (std::string::const_iterator ci = fullname.begin(); 
			 ci != fullname.end(); ++ci) {
			if (*ci == ':' && bracket_balance == 0) {
				PyList_Append(pylist, 
							  PyString_FromStringAndSize((char*)&*start,
														 int(ci - start)));
				++ci;
				start = ci + 1;
			}
			else if (*ci == '<') ++bracket_balance;
			else if (*ci == '>') --bracket_balance;
		}

		// - append last part of name
		PyList_Append(pylist, PyString_FromString((char*)&*start));

		return pylist;
	}


	void insertIntoNamespace(const std::string& modulename,
							 PyObject *module,
							 PyObject *namelist,
							 PyObject *object)
	{
		assert(PyModule_Check(module));
		assert(PyList_Check(namelist));

		int n_names = PyList_Size(namelist);
		int last = n_names - 1;

		PyObject *rootmodule = module;
		std::string fullname = modulename;
		static PyMethodDef empty[] = { { 0 } };

		for (int nameindex = 0; nameindex < last; ++nameindex) {
			PyObject *pyname = PyList_GetItem(namelist, nameindex);
			// - create a submodule in current module
			char *submodulename = PyString_AsString(pyname);
			std::string submodule_fullname = fullname + "." + submodulename;
			// - try "import" first, "init" new module if fails
			char *c_submodulename = (char*)submodule_fullname.c_str();
			PyObject *submodule = PyImport_ImportModule(c_submodulename);
			if (submodule == NULL) {
				PyErr_Clear();
				submodule = PyObject_GetAttr(module, pyname);
				if (submodule == NULL) {
					PyErr_Clear();
					// - create a module object
					submodule = Py_InitModule(c_submodulename, empty);
					// - place new module in current module's dict
					assert(PyModule_Check(module));
					PyModule_AddObject(module, submodulename, submodule);
				}
				else if (!(PyModule_Check(submodule) || 
						   ClassObject_Check(submodule))) {
					// - container object is not a module or class,
					//   fall back to root module.
					module = rootmodule;
					break;
				}
			}
			assert(submodule && submodule->ob_type);
			// - descend to submodule
			module = submodule;
			fullname = submodule_fullname;
		}

		// Now insert the object in the proper submodule
		PyObject *pylast = PyList_GetItem(namelist, last);
		PyObject *dict = PyModule_Check(module) ? PyModule_GetDict(module)
		                                        : ClassObject_GetDict(module);
		PyDict_SetItem(dict, pyowned(pylast), pyowned(object));
	}

	void insertIntoNamespace(const std::string& modulename,
							 PyObject *module, 
							 const std::string& cppname,
							 PyObject *object)
	{
		// Split original C++ name by ::
		PyObject *pysplit = splitName(cppname);

		insertIntoNamespace(modulename, module, pysplit, object);

		Py_XDECREF(pysplit);
	}

    PyObject *getPrimitiveType(Handle<Namespace> &namespc,
                               const std::string &type_name)
    {
        std::string type = type_name;
		
		// remove the unnecessary modifiers (just const for now)
		if (type.substr(0,6) == "const ") {
			type.erase(type.begin(),
			                  type.begin() + 6);
		}

        namespc->unalias(type);

		// identify the argument
		if      (type == "int")      return (PyObject*)&PyInt_Type;
		else if (type == "float")    return (PyObject*)&PyFloat_Type;
		else if (type == "long")     return (PyObject*)&PyLong_Type;
		else if (type == "long long")       return pylong_long;
		else if (type == "char")            return pychar;
		else if (type == "double")          return pydouble;
		else if (type == "unsigned int")    return pyunsigned_int;
		else if (type == "unsigned long")   return pyunsigned_long;
		else if (type == "unsigned long long")
			                                       return pyunsigned_long_long;
		else if (type == "unsigned char")   return pyunsigned_char;
		else if (type == "signed char")     return pysigned_char;
		else if (type == "char *")    return (PyObject*)&PyString_Type;
        else
                // not a primitive type
                throw LookupFailureException();
    }

    PyObject *getBuiltinType(Handle<Namespace> &namespc,
                               const std::string &type_name)
    {
        std::string type = type_name;
		
		// remove the unnecessary modifiers (just const for now)
		if (type.substr(0,6) == "const ") {
			type.erase(type.begin(),
			                  type.begin() + 6);
		}

        namespc->unalias(type);

		// identify the argument
		if      (type == "int")      return (PyObject*)&PyInt_Type;
		else if (type == "float")    return (PyObject*)&PyFloat_Type; 
		else if (type == "long")     return (PyObject*)&PyLong_Type; 
		else if (type == "long long")       return (PyObject*)&PyLong_Type;
		else if (type == "char")            return (PyObject*)&PyString_Type;
		else if (type == "double")          return (PyObject*)&PyFloat_Type;
		else if (type == "unsigned int")    return (PyObject*)&PyInt_Type;
		else if (type == "unsigned long")   return (PyObject*)&PyLong_Type;
		else if (type == "unsigned long long")
			                                       return (PyObject*)&PyLong_Type;
		else if (type == "unsigned char")   return (PyObject*)&PyString_Type;
		else if (type == "signed char")     return (PyObject*)&PyString_Type;
		else if (type == "char *")    return (PyObject*)&PyString_Type;
        else
                throw LookupFailureException();


    }




	PyObject *identifyTemplateArgument(Handle<Namespace> namespc, 
                                       PythonFrontend& fe,
									   const std::string& templateargname)
	{
		std::string templatearg = templateargname;
		
		// remove the unnecessary modifiers (just const for now)
		if (templatearg.substr(0,6) == "const ") {
			templatearg.erase(templatearg.begin(),
			                  templatearg.begin() + 6);
		}

        try {
            // see if it's a primitive type
            return getPrimitiveType(namespc, templatearg);
        } catch(LookupFailureException &) {
			return (PyObject*)fe.getClassObject(templatearg);
        }
	}

	/**
	 * Runs a list of user-defined detectors, and if one succeeds, returns
	 * the result.
	 *
	 * @param type_list list of UserDefinedTranslator instances
	 * @param element the element to be detected
	 */
	Handle<TypeOfArgument> detectUserDefined
		(std::list<Handle<UserDefinedTranslator> > type_list, 
		 scripting_element element)
	{
		Handle<TypeOfArgument> user;
		typedef std::list<Handle<UserDefinedTranslator> > userdefinedtypelist;

		for (userdefinedtypelist::const_iterator userit = type_list.begin();
			 userit != type_list.end(); ++userit) {
			if (user = (*userit)->detectType(element))
				return user;
		}

		return Handle<TypeOfArgument>();
	}
}


/**
 * PythonFrontend constructor.
 */
PythonFrontend::PythonFrontend()
{
	m_lowLevel = new PythonLowLevel();
	m_interceptor = new PythonInterceptor();
	m_errorHandler = new PythonErrorHandler();
}

PythonFrontend::~PythonFrontend()
{
	delete m_lowLevel;
	delete m_interceptor;
	delete m_errorHandler;
}

/**
 * Commit general initializations
 * (Currently does nothing).
 */
void PythonFrontend::initialize() const
{
	// Create some primitive conversions
	Handle<Conversion> hlong2int(new TrivialConversion);
	Handle<Conversion> hlong2uint(new TrivialConversion);
	Handle<Conversion> hlong2short(new TrivialConversion);
	Handle<Conversion> hlong2ushort(new TrivialConversion);
	Handle<Conversion> hlong2ulong(new TrivialConversion);
	Handle<Conversion> hlong2bool(new TrivialConversion);
	Handle<Conversion> hint2longlong(new TrivialConversion);
	Handle<Conversion> hint2ulonglong(new TrivialConversion);
	Handle<Conversion> hpylong2longlong(new TrivialConversion);
	Handle<Conversion> hpylong2ulonglong(new TrivialConversion);
	Handle<Conversion> hbool2long(new TrivialConversion);
	Handle<Conversion> hdouble2float(new TrivialConversion);
	Handle<Conversion> hlong2double(new IntToFloatConversion);
	Handle<Conversion> hchar2uchar(new TrivialConversion);
	Handle<Conversion> hchar2string(new TrivialConversion);
	Handle<Conversion> hpascal2cstring(new PascalStringToCStringConversion);
	Handle<Conversion> hlist2element(new TrivialConversion);
	Handle<Conversion> hlongtruncate(new LongLongTruncate);

	hlong2int        ->setSourceType(ArgumentLong);
	hlong2int        ->setTargetType(ArgumentInt);
	hlong2uint       ->setSourceType(ArgumentLong);
	hlong2uint       ->setTargetType(ArgumentUInt);
	hlong2short      ->setSourceType(ArgumentLong);
	hlong2short      ->setTargetType(ArgumentShort);
	hlong2ushort     ->setSourceType(ArgumentLong);
	hlong2ushort     ->setTargetType(ArgumentUShort);
	hlong2ulong      ->setSourceType(ArgumentLong);
	hlong2ulong      ->setTargetType(ArgumentULong);
	hlong2bool       ->setSourceType(ArgumentLong);
	hlong2bool       ->setTargetType(ArgumentBoolean);
	hint2longlong    ->setSourceType(ArgumentInt);
	hint2longlong    ->setTargetType(ArgumentLongLong);
	hint2ulonglong   ->setSourceType(ArgumentInt);
	hint2ulonglong   ->setTargetType(ArgumentULongLong);
	hpylong2longlong ->setSourceType(ArgumentPythonLong);
	hpylong2longlong ->setTargetType(ArgumentLongLong);
	hpylong2ulonglong->setSourceType(ArgumentPythonLong);
	hpylong2ulonglong->setTargetType(ArgumentULongLong);
	hbool2long       ->setSourceType(ArgumentBoolean);
	hbool2long       ->setTargetType(ArgumentLong);
	hdouble2float    ->setSourceType(ArgumentDouble);
	hdouble2float    ->setTargetType(ArgumentFloat);
	hlong2double     ->setSourceType(ArgumentLong);
	hlong2double     ->setTargetType(ArgumentDouble);
	hchar2uchar      ->setSourceType(ArgumentChar);
	hchar2uchar      ->setTargetType(ArgumentUChar);
	hchar2string     ->setSourceType(ArgumentChar);
	hchar2string     ->setTargetType(ArgumentPascalString);
	hpascal2cstring  ->setSourceType(ArgumentPascalString);
	hpascal2cstring  ->setTargetType(ArgumentCString);
	hlist2element    ->setSourceType(ArgumentPythonList);
	hlist2element    ->setTargetType(ArgumentScriptingElement);
	hlongtruncate    ->setSourceType(ArgumentPythonLong);
	hlongtruncate    ->setTargetType(ArgumentLong);

	hint2longlong->setWeight(Conversion::Weight(0,1,0,0));
	hint2ulonglong->setWeight(Conversion::Weight(0,1,0,0));
	hbool2long->setWeight(Conversion::Weight(0,1,0,0));

	ConversionTableSingleton::getInstance()->registerConversion(hlong2int);
	ConversionTableSingleton::getInstance()->registerConversion(hlong2uint);
	ConversionTableSingleton::getInstance()->registerConversion(hlong2short);
	ConversionTableSingleton::getInstance()->registerConversion(hlong2ushort);
	ConversionTableSingleton::getInstance()->registerConversion(hlong2ulong);
	ConversionTableSingleton::getInstance()->registerConversion(hint2longlong);
	ConversionTableSingleton::getInstance()->registerConversion(hint2ulonglong);
	ConversionTableSingleton::getInstance()->registerConversion(hpylong2longlong);
	ConversionTableSingleton::getInstance()->registerConversion(hpylong2ulonglong);
	ConversionTableSingleton::getInstance()->registerConversion(hlong2bool);
	ConversionTableSingleton::getInstance()->registerConversion(hbool2long);
	ConversionTableSingleton::getInstance()->registerConversion(hdouble2float);
	ConversionTableSingleton::getInstance()->registerConversion(hlong2double);
	ConversionTableSingleton::getInstance()->registerConversion(hchar2uchar);
	ConversionTableSingleton::getInstance()->registerConversion(hchar2string);
	ConversionTableSingleton::getInstance()
		->registerConversion(hpascal2cstring);
	ConversionTableSingleton::getInstance()
		->registerConversion(hlist2element);
	ConversionTableSingleton::getInstance()->registerConversion(hlongtruncate);
}

/**
 * Reveal the type of a given PyObject from its Python type.
 */
Handle<TypeOfArgument> PythonFrontend::detectType(scripting_element element)
	const
{
	PyObject *object = reinterpret_cast<PyObject*>(element);
	PyObject *obtype = PyObject_Type(object);
	Handle<TypeOfArgument> user;

	if (object == Py_True || object == Py_False)
		return ArgumentBoolean;
	else if (PyInt_Check(object))
		return ArgumentLong;
	else if (PyFloat_Check(object))
		return ArgumentDouble;
	else if (PyString_Check(object))
		return (PyString_Size(object)==1)? ArgumentChar : ArgumentPascalString;
	else if (PyList_Check(object))
		return ArgumentPythonList;
	else if (PyTuple_Check(object))
		return ArgumentPythonTuple;
	else if (PyDict_Check(object))
		return ArgumentPythonDict;
	else if (PyLong_Check(object))
		return ArgumentPythonLong;
	else if (ClassObject_Check(obtype)) {
		// - instance object
		Handle<Class> clas = ((ClassObject*)obtype)->getUnderlying();
		return clas->getRefArg();
	}
	else if (PyObject_Type(obtype) == (PyObject*)&EnumeratedTypeTypeObject) {
		// - enumerated constant object
		Handle<EnumeratedConstant> enumconst = 
			((EnumeratedConstantObject*)object)->getUnderlying();
		return enumconst->getType()->getArg();
	}
	else if (user = detectUserDefined(m_userTypes, element)) {
		// - user-defined type object
		return user;
	}
	else if (PyInstance_Check(object) || _PyObject_GetDictPtr(object) != 0
			 || PyCObject_Check(object)) {
		return ArgumentScriptingElement;
	}
	else if (obtype == (PyObject*)&AddressTypeObject) {
		Handle<Address> address = ((AddressObject*)object)->getUnderlying();
		return address->getPointerType();
	}
	else
		throw UnsupportedInterfaceException();
}

/**
 * Gives value insight.
 * The only insight currently supplied is for lists and dictionaries:
 * in this case, the frontend will return the type of the first element.
 * In a list this is the first element of the list.
 * In a dictionary this is a tuple containing the first key and it's value.
 */
Insight PythonFrontend::detectInsight(scripting_element element) const
{
	PyObject *object = reinterpret_cast<PyObject*>(element);
	Insight insight;

	if (PyList_Check(object) && PyList_Size(object) > 0) {
		PyObject *first = PyList_GetItem(object, 0);
		//insight.i_ptr = PyObject_Type(first);
        Py_XINCREF(first);
		insight.i_ptr = first;
	}
	else if (PyDict_Check(object) && PyDict_Size(object) > 0) {
		PyObject *items = PyDict_Items(object);
		PyObject *first = PyList_GetItem(items, 0);
		PyObject *types = PyList_New(2);
		PyList_SetItem(types, 0, PyObject_Type(PyTuple_GetItem(first, 0)));
		PyList_SetItem(types, 1, PyObject_Type(PyTuple_GetItem(first, 1)));
		Py_XDECREF(items);
		insight.i_ptr = types;
	}
	else if (PyLong_Check(object)) {
		long long value = PyLong_AsLongLong(object);
		if (value < 1ll << (sizeof(long)*8))
			insight.i_long = sizeof(long);
		else
			insight.i_long = sizeof(long long);
	}
	else {
		insight.i_ptr = 0;
	}

	return insight;
}

/**
 * Statically detect a type represented by a Python PyTypeObject.
 */
Handle<TypeOfArgument> PythonFrontend::detectType(struct _typeobject *pytype)
	const
{
	if (pytype == &PyInt_Type)
		return ArgumentInt;
	else if (pytype == &PyString_Type)
		return ArgumentCString;
	else if (ClassObject_Check((PyObject*)pytype)) {
		return ((ClassObject*)pytype)->getUnderlying()->getRefArg();
	}
	else
		throw UnsupportedInterfaceException();
}


/**
 * Creates a Python adapter for a specific type.
 */
Handle<Adapter> PythonFrontend::giveAdapterFor(const TypeOfArgument& type)
	const
{
	Type basetype = type.basetype();

	if (type.isPointer()) {
		Handle<TypeOfArgument> pointedType = 
			TypeOfArgument::handleMap.acquire(type.pointed());
		if (pointedType)
			return Handle<Adapter>(new AddressAdapter(pointedType));
		else if (basetype.category == TYPE_CATEGORY_EXTENDED
				 && basetype.spec == TYPE_EXTENDED_VOID)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<void*, PyCObjectTraits>);
		else
			throw UnsupportedInterfaceException();
	}
	if (basetype.category == TYPE_CATEGORY_INTRINSIC) {
		if (basetype.spec == TYPE_INTRINSIC_INT)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<int, PyIntTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_UINT)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<unsigned int, PyIntTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_LONG)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<long, PyIntTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_LONG_LONG)
			return Handle<Adapter>
				(new AllocatedPrimitivePythonAdapter<long long,
				                                   PyLongTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_ULONG)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<unsigned long, 
			                                       PyIntTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_ULONG_LONG)
			return Handle<Adapter>
				(new AllocatedPrimitivePythonAdapter<unsigned long long,
				                                   PyLongTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_SHORT)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<short, PyIntTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_USHORT)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<unsigned short, 
			                                       PyIntTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_CHAR)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<char, Py1CharStringTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_UCHAR)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<unsigned char,
				                                 Py1CharStringTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_SCHAR)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<signed char,
				                                 Py1CharStringTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_BOOL)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<bool, PyBoolTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_FLOAT)
			return Handle<Adapter>
				(new SmallPrimitiveReinterpretPythonAdapter
                                                <float, PyFloatSmallTraits>());
		else if (basetype.spec == TYPE_INTRINSIC_DOUBLE)
			return Handle<Adapter>
				(new AllocatedPrimitivePythonAdapter<double, PyFloatTraits>());
		else
			throw UnsupportedInterfaceException();
	}
	else if (basetype.category == TYPE_CATEGORY_EXTENDED) {
		if (basetype.spec == TYPE_EXTENDED_CSTRING)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<const char *, PyStringTraits>
				 ());
		else if (basetype.spec == TYPE_EXTENDED_PASCALSTRING)
			return Handle<Adapter>(new PascalStringAdapter);
		else if (basetype.spec == TYPE_EXTENDED_ELEMENT)
			return Handle<Adapter>(new PyObjectAdapter);
		else
			throw UnsupportedInterfaceException();
	}
	else if (basetype.category == TYPE_CATEGORY_USERDEFINED) {
		if (basetype.spec == TYPE_USERDEFINED_OBJECT) {
			// - find the class object
			ClassObject *classobj = getClassObject(basetype.objclass);
			// - create instance adapter
			bool owned = (&type == &*(basetype.objclass->getPtrArg()));
			return Handle<Adapter>(new InstanceAdapter(classobj,
													   owned));
		}
		else if (basetype.spec == TYPE_USERDEFINED_ENUM) {
			// - find the enum object
			EnumeratedTypeObject *enumobj = 
				getEnumeratedTypeObject(basetype.objenum);
			// - create enum adapter
			return Handle<Adapter>(new EnumeratedAdapter(enumobj));
		}
		else
			throw UnsupportedInterfaceException();
	}
	else
		throw UnsupportedInterfaceException();
}

/**
 * Gets the names of all the functions and classes in the library and
 * exposes them as a Python module.
 */
void PythonFrontend::exposeLibrary(const Library& newcomer)
{
	static PyMethodDef meth[] = { { 0 } };
	PyObject *module = Py_InitModule((char*)(newcomer.name().c_str()), meth);
	PyObject *dict = PyModule_GetDict(module);

	Handle<Namespace> globals = newcomer.globalNamespace();

	typedef Handle<Namespace::NameIterator> hiterator;
	// Expose classes
	for (hiterator cname_iter = globals->enumerateClasses();
		 !cname_iter->done(); cname_iter->next()) {
		// Create a class object in the module
		std::string name = cname_iter->value();
		Handle<Class> clas = globals->lookupClass(name);
		ClassObject *pyclass = getClassObject(clas);
		pyclass->inModule(module);
		insertIntoNamespace(newcomer.name(), module, name, (PyObject*)pyclass);
		// Register class in classes map
		m_classes[&*clas]     = pyclass;
		m_classesByName[name] = pyclass;
		// Apply implicit enhancements to this class
		Protocol::autoEnhance(pyclass);
	}
	// Expose global functions and global variables
	for (hiterator fname_iter = globals->enumerateFunctions();
		 !fname_iter->done(); fname_iter->next()) {
		// Look the function up in the global namespace
		std::string name = fname_iter->value();
		Handle<Callable> routine = globals->lookupFunction(name);
		if (name.substr(0, DATAMEMBER_PREFIX.size()) == DATAMEMBER_PREFIX) {
			// Create a variable in the module by calling the function
			std::string varname = name.substr(DATAMEMBER_PREFIX.size());
			ActualArgumentList noargs;
			PyObject *pyvar = (PyObject*)routine->call(noargs);
			insertIntoNamespace(newcomer.name(), module, varname, pyvar);
		}
		else {
			// Create a function object in the module
			FunctionObject *pyroutine = new FunctionObject(routine);
			pyroutine->setName(name);
			pyroutine->inModule(module);
			insertIntoNamespace(newcomer.name(), module, name, pyroutine);
			// Register in function map
			m_functionsByName[name] = pyroutine;
		}
	}
	// Expose enumerated types
	for (hiterator ename_iter = globals->enumerateEnums();
		 !ename_iter->done(); ename_iter->next()) {
		std::string name = ename_iter->value();
		Handle<EnumeratedType> enumtype = globals->lookupEnum(name);
		EnumeratedTypeObject *pyenumtype = getEnumeratedTypeObject(enumtype);
		insertIntoNamespace(newcomer.name(), module, 
							name, (PyObject*)pyenumtype);
		// Get the list of constants belonging to this type
		std::vector<Handle<EnumeratedConstant> > enumconstants =
			listOfConstants(enumtype);
		// Create objects representing enum constants
		for (std::vector<Handle<EnumeratedConstant> >::iterator ci =
				 enumconstants.begin(); ci != enumconstants.end(); ++ci) {
			PyObject *obj = new EnumeratedConstantObject(pyenumtype, *ci);
			PyObject *namespaces = splitName(name);
			PyList_SetItem(namespaces,
			               PyList_Size(namespaces) - 1,
			               PyString_FromString((*ci)->getLiteral().c_str()));
			insertIntoNamespace(newcomer.name(), module, 
								namespaces, obj);
		}
	}
	// Apply aliases
	for (hiterator alias_iter = globals->enumerateAliases();
		 !alias_iter->done(); alias_iter->next()) {
		try {
			std::string name = alias_iter->value();
			
            Handle<Class> aliased = globals->lookupClass(name);
			ClassObject *pyaliased = m_classes[&*aliased];
			insertIntoNamespace(newcomer.name(), module, name,
								pyowned((PyObject*)pyaliased));
			// Register class in classes map by its alias as well
			m_classesByName[name] = pyaliased;
		}
		catch (LookupFailureException& ) {
            // this could be a primitive type
            try {
                    std::string name = alias_iter->value();
                    // FIXME: XXX:
                    // if desired to use a python built-in type (type<'int'>, etc)
                    // this call should be replaced by getBuiltinType
                    PyObject *prim_type = getPrimitiveType(globals, name);
                    insertIntoNamespace(newcomer.name(), module, name,
                                    pyowned(prim_type));
                    continue;
            } catch(LookupFailureException &) {
                    // not a primitive, fallthrough to failure scenario
            } 

			// ignore this alias
		}
	}
	// Expose templates
	for (hiterator tname_iter = globals->enumerateClasses();
		 !tname_iter->done(); tname_iter->next()) {
		// Create or access a template object if necessary
		std::string classname = tname_iter->value(), templatename;
		TemplateObject *pytemplate = exposeTemplate(globals, T_CLASS, classname,
													templatename);
		if (pytemplate)
			insertIntoNamespace(newcomer.name(), module, templatename, 
								(PyObject*)pytemplate);
	}
	for (hiterator tfname_iter = globals->enumerateFunctions();
		 !tfname_iter->done(); tfname_iter->next()) {
		// Create or access a template object if necessary
		std::string funcname = tfname_iter->value(), templatename;
		TemplateObject *pytemplate = exposeTemplate(globals, T_FUNCTION, funcname,
													templatename);
		if (pytemplate)
			insertIntoNamespace(newcomer.name(), module, templatename, 
								(PyObject*)pytemplate);
	}
}

/**
 * Checks whether a given class-name suggests a template instantiation.
 *
 * @param classname a literal name
 * @param templatename set to the name of the template in case one was found
 * @param templateargs set to the names of all template arguments in case a
 * template was found
 * @return <b>true</b> if 'classname' is a template instance, in which case
 * 'templatename' is set to the full name of the appropriate template.
 * <b>false</b> otherwise.
 */
bool PythonFrontend::getTemplateName(const std::string& classname,
                                     std::string& templatename,
                                     std::vector<std::string>& templateargs)
{
	// find the first <
	int lt = classname.find('<');

	// if it wasn't found, this is not a template
	if (lt == -1) {
		return false;
	}
	else {
		// find the last > that closes this one
		int gt = classname.rfind('>');
		// if there is no final bracket (this cannot possibly happen)
		assert(gt < classname.length());
		// if this is not the last character, then this could be a class inside
		// a template class, such as std::vector<int>::iterator
		if (gt != (classname.length() - 1)) {
			return false;
		}
		
		// extract the template name
		templatename = classname.substr(0, lt);

		// extract all of the arguments by taking all characters between the <>
		std::string templatearg = 
			classname.substr(lt + 2, gt - lt - 3) + ",";
		// split the string by commas
		int parenthesisCount;
		while (templatearg.size()) {
			// find the next comma
			int i;
			parenthesisCount = 0;
			for (i = 0; i < templatearg.size(); ++i) {
				if ((templatearg[i] == ',') && (parenthesisCount == 0)) {
					break;
				}
				if (templatearg[i] == '<') {
					parenthesisCount++;
				}
				if (templatearg[i] == '>') {
					parenthesisCount--;
				}
			}

			// get the next template argument
			std::string currrentarg = templatearg.substr(0,i);
			templateargs.push_back(currrentarg);

			// erase the current argument from the arg string
			templatearg.erase(templatearg.begin(), templatearg.begin() + i + 1);
		}
		
		return true;
	}
}

/**
 * Adds a template instance to the dictionary object associated with that
 * template. If the dictionary does not exist yet, it is created and added to
 * the template registry.
 *
 * @param kind either T_CLASS or T_FUNCTION according to the group to which
 * this template instance belongs: class template or function template
 * @param elemname the full name of the template instance in normal form,
 * for example "std::vector< int >".
 * @param templatename assigned with the string comprising of the left portion
 * of the template specification, for example "std::vector".
 * @return the template element, basically a Python dictionary object.
 * @note it is safe to call this function with a classname which is not a
 * template instance at all; in that case nothing will be done and the return
 * value will be <b>NULL</b>.
 */
TemplateObject *PythonFrontend::exposeTemplate(Handle<Namespace> &namespc, 
                                               TemplateKind kind,
											   const std::string& elemname,
											   std::string& templatename)
{
	std::vector<std::string> templateargs;
	TemplateObject *pytemplate;

	// Get the template name
	if (getTemplateName(elemname, templatename, templateargs)) {
		// Look whether the template is already registered
		templatenameassoc::const_iterator templfind = 
			m_templatesByName.find(templatename);
		
		if (templfind == m_templatesByName.end()) {
			m_templatesByName[templatename] = 
				pytemplate = (TemplateObject*)PyDict_New();
		}
		else {
			pytemplate = templfind->second;
		}

		// Locate all classes being used as the template arguments
		PyObject *pyarguments = NULL;
		// If this is only one item, just use it instead of the tuple
		if (templateargs.size() == 1) {
			pyarguments = identifyTemplateArgument(namespc, *this, templateargs[0]);
			Py_XINCREF(pyarguments);
		}
		// If there are more than one argument, use a tuple
		else {
			pyarguments = PyTuple_New(templateargs.size());
			for (size_t i = 0; i < templateargs.size(); ++i) {
				PyObject *pyargument = 
					identifyTemplateArgument(namespc, *this, templateargs[i]);
				if (!pyargument) {
					return NULL;
				}
				Py_XINCREF(pyargument);
				PyTuple_SetItem(pyarguments, i, pyargument);
			}
		}

		if (pyarguments) {
			// Update the dictionary with the current element
			PyObject_SetItem((PyObject*)pytemplate,
							 pyarguments,
							 (PyObject*)getRegisteredObject(kind, elemname));
			Py_XDECREF(pyarguments);
		}

		return pytemplate;
	}
	else {
		return NULL;
	}
}

void *PythonFrontend::getRegisteredObject(TemplateKind kind,
										  const std::string& name)
{
	switch (kind) {
	case T_CLASS:    return getClassObject(name);
	case T_FUNCTION: return getFunctionObject(name);
	};
}

/**
 * Increments the reference count of the object.
 */
scripting_element PythonFrontend::duplicateReference(scripting_element element)
{
	PyObject *py = reinterpret_cast<PyObject*>(element);
	Py_XINCREF(py);
	return element;
}

/**
 * Decreases the reference count of the object. When Python detects that the
 * object is no longer in use, it is deallocated by the garbage collection
 * mechanism, invoking the approperiate __dealloc__ method and causing actual
 * deletion of the object.
 */
void PythonFrontend::release(scripting_element element)
{
	PyObject *object = reinterpret_cast<PyObject*>(element);
	Py_XDECREF(object);
}

void PythonFrontend::bond(scripting_element master, scripting_element slave)
{
    if (!InstanceObject_Check((PyObject*)master) || !InstanceObject_Check((PyObject*)slave)) {
        return;
    }

    InstanceObject *islave  = reinterpret_cast<InstanceObject*>(slave);
    InstanceObject *imaster = reinterpret_cast<InstanceObject*>(master);

    // XXX: Should imaster's bondage be assigned to null
    // not doing it could potentially lead to double-frees
    islave->getUnderlying()->bond(imaster->getUnderlying());
}

/**
 * Returns the low level interface for low level function calls.
 */
const LowLevel& PythonFrontend::getLowLevel() const
{
	return *m_lowLevel;
}

/**
 * Returns the interceptor aspect for this frontend.
 */
const Interceptor& PythonFrontend::getInterceptor() const
{
	return *m_interceptor;
}

/**
 * Retruns the error handler for this frontend.
 */
ErrorHandler& PythonFrontend::getErrorHandler()
{
	return *m_errorHandler;
}

/**
 * Finds a Python::ClassObject via a Robin::Class reference.
 */
ClassObject *PythonFrontend::getClassObject(Handle<Robin::Class> clas) const
{
	classassoc::const_iterator classfind = 
		m_classes.find(&*clas);
	// If class exists in map, return associated value.
	// Otherwise, create a new class object and register it.
	if (classfind == m_classes.end()) {
		return (m_classes[&*clas] = m_classesByName[clas->name()] 
					= new ClassObject(clas));
	}
	else {
		assert(classfind->second != NULL);
		return classfind->second;
	}
}

/**
 * Finds a Python::ClassObject by literal name.
 * Returns <b>NULL</b> if none exist.
 */
ClassObject *PythonFrontend::getClassObject(const std::string& name) const
{
	classnameassoc::const_iterator classfind = 
		m_classesByName.find(name);
	// If class exists in map, return associated value.
	if (classfind == m_classesByName.end()) {
		return NULL;
	}
	else {
		assert(classfind->second != NULL);
		return classfind->second;
	}
}

/**
 * Finds a Python::FunctionObject by literal name.
 * Returns <b>NULL</b> if none exist.
 */
FunctionObject *PythonFrontend::getFunctionObject(const std::string& name) 
	const
{
	funcnameassoc::const_iterator funcfind = m_functionsByName.find(name);
	// If class exists in map, return associated value.
	if (funcfind == m_functionsByName.end()) {
		return NULL;
	}
	else {
		assert(funcfind->second != NULL);
		return funcfind->second;
	}
}

/**
 * Finds a Python::EnumeratedTypeObject via a Robin::EnumeratedType reference.
 * Returns <b>NULL</b> if none exist.
 */
EnumeratedTypeObject *PythonFrontend
	::getEnumeratedTypeObject(Handle<EnumeratedType> enumtype) const
{
	enumassoc::const_iterator enumfind = m_enums.find(&*enumtype);
	// If class exists in map, return associated value.
	if (enumfind == m_enums.end()) {
		// - create a new enum object for that type
		return (m_enums[&*enumtype] = new EnumeratedTypeObject(enumtype));
	}
	else {
		assert(enumfind->second != NULL);
		return enumfind->second;
	}
}

/**
 * Finds a Python::TemplateObject by literal name.
 *
 * @param name the full name of the class template
 * @return a TemplateObject instance matching that name if one exists,
 * <b>NULL</b> if none exist.
 */
TemplateObject *PythonFrontend::getTemplateObject(const std::string& name)
	const
{
	templatenameassoc::const_iterator templfind = 
		m_templatesByName.find(name);
	// If template exists in map, return associated value.
	if (templfind == m_templatesByName.end()) {
		return NULL;
	}
	else {
		assert(templfind->second != NULL);
		return templfind->second;
	}
}

/**
 * Allows user to pre-set a template object in such a way that every instance
 * of the specified template is inserted into it using PyMapping_SetItem,
 * thus allowing enhanced control over the generation of instances.
 *
 * @param name the full name of the class template
 * @param pytemplate a Python object which extends PyDict_Type
 */
void PythonFrontend::setTemplateObject(const std::string& name,
									   TemplateObject *pytemplate)
{
	// If there is an existing dictionary associated with that template,
	// copy all the values from the old dictionary to the new dictionary
	// before destroying the previous store
	TemplateObject *existing = m_templatesByName[name];

	if (existing) {
		PyObject *items = PyMapping_Items((PyObject*)existing);
		if (items != NULL) {
			long nitems = PyList_Size(items);
			assert(nitems >= 0);
			for (long i = 0; i < nitems; ++i) {
				PyObject *key, *value;
				if (PyArg_ParseTuple(PyList_GetItem(items, i), "OO", 
									 &key, &value)) {
					PyObject_SetItem((PyObject*)pytemplate, key, value);
				}
			}
		}
		Py_DECREF(existing);
	}

	// Register the new template object in the templates map
	Py_INCREF(pytemplate);
	m_templatesByName[name] = pytemplate;
}


/**
 * Registers a new type using a user-defined translator.
 *
 * @param translator a UserDefinedTranslator implementation which is capable
 * of translating the new type
 */
void PythonFrontend::addUserDefinedType(Handle<UserDefinedTranslator> 
										translator)
{
	m_userTypes.push_front(translator);
}


ByTypeTranslator::ByTypeTranslator(PyTypeObject *pytype)
	: m_type(new TypeOfArgument(TYPE_CATEGORY_USERDEFINED, 
								TYPE_USERDEFINED_OBJECT)),
	  m_pytype(pytype)
{
}

Handle<TypeOfArgument> ByTypeTranslator::detectType(scripting_element element)
{
	PyObject *pyelement = (PyObject*)element;
	if (PyObject_Type(pyelement) == (PyObject*)m_pytype) {
		return m_type;
	}
	else {
		return Handle<TypeOfArgument>();
	}
}



} // end of namespace Robin::Python

} // end of namespace Robin

