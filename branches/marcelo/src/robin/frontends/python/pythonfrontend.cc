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
#include "pyhandle.h"

// Robin includes
#include <robin/debug/assert.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/namespace.h>
#include <robin/reflection/library.h>
#include <robin/reflection/class.h>
#include <robin/reflection/address.h>
#include <robin/reflection/enumeratedtype.h>
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/instance.h>
#include <robin/reflection/fundamental_conversions.h>
#include <robin/reflection/pointer.h>
#include <robin/reflection/const.h>
#include <robin/registration/mechanism.h>

// Python Frontend includes
#include "pythonadapters.h"
#include "pythonobjects.h"
#include "pythonconversions.h"
#include "types/numericsubtypes.h"
#include "types/listrobintype.h"
#include "types/dictrobintype.h"


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
extern PyObject *c_int;
extern PyObject *c_long;
extern PyObject *pyunsigned_int;
extern PyObject *pyunsigned_long_long;
extern PyObject *pyunsigned_char;
extern PyObject *pysigned_char;
extern PyObject *c_short;
extern PyObject *c_ushort;
extern PyObject *c_string;
extern PyObject *c_float;

namespace {

	PyObject *pyowned(PyObject *object)
	{
		Py_XINCREF(object);
		return object;
	}


	/**
	 * It generates from a C++ full type name a list of
	 * python object names which represents how to search
	 * for that object.
	 * For example if the type is 'std::vector', it will
	 * generate the python scripts 'std', 'vector'.
	 * For templated types it will
	 * hide the type in a module called '_templates'.
	 * For example 'std::vector<int>' will be placed in
	 *  'std', '_templates', 'vector<int>'
	 */
	PyObject *generatePythonModulePath(const std::string& fullname)
	{
		PyObject *pylist = PyList_New(0);

		unsigned int bracket_balance = 0;
		bool wastemplated = false;
		std::string::const_iterator start = fullname.begin();

		for (std::string::const_iterator ci = fullname.begin(); 
			 ci != fullname.end(); ++ci) {
			if (*ci == ':' && bracket_balance == 0) {
				PyList_Append(pylist, 
							PyString_FromStringAndSize(const_cast<char*>(&*start),
									  Py_ssize_t(ci - start)));
				++ci;
				start = ci + 1;
			}
			else if (*ci == '<')
			{
				++bracket_balance;
				wastemplated = true;
			}
			else if (*ci == '>')
			{
				--bracket_balance;
				if (wastemplated) {
					PyList_Append(pylist, PyString_FromString("template_classes_robin"));
					wastemplated = false;
				}
			}
		}

		// - append last part of name
		PyList_Append(pylist, PyString_FromString(const_cast<char*>(&*start)));

		return pylist;
	}

	/*
	 * Adss a python object to a module (or a submodule if using namelist).
	 * Only borrows the reference to object (because it used PyDict_SetItem
	 */
	PyReferenceBorrow<PyObject> insertIntoNamespace(const std::string& modulename,
							 PyObject *module,
							 PyObject *namelist,
							 PyReferenceBorrow<PyObject> object)
	{
		assert_true(PyModule_Check(module));
		assert_true(PyList_Check(namelist));

		Py_ssize_t n_names = PyList_Size(namelist);
		Py_ssize_t last = n_names - 1;

		PyObject *rootmodule = module;
		std::string fullname = modulename;
		static PyMethodDef empty[] = { { 0 } };

		for (Py_ssize_t nameindex = 0; nameindex < last; ++nameindex) {
			PyObject *pyname = PyList_GetItem(namelist, nameindex);
			// - create a submodule in current module
			char *submodulename = PyString_AsString(pyname);
			std::string submodule_fullname = fullname + "." + submodulename;
			// - try "import" first, "init" new module if fails
			char *c_submodulename = const_cast<char*>(submodule_fullname.c_str());
			PyObject *submodule = PyImport_ImportModule(c_submodulename);
			if (submodule == NULL) {
				PyErr_Clear();
				submodule = PyObject_GetAttr(module, pyname);
				if (submodule == NULL) {
					PyErr_Clear();
					// - create a module object
					submodule = Py_InitModule(c_submodulename, empty);
					// - place new module in current module's dict
					assert_true(PyModule_Check(module));
					Py_XINCREF(submodule);
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
			assert_true(submodule && submodule->ob_type);
			// - descend to submodule
			module = submodule;
			fullname = submodule_fullname;
		}

		// Now insert the object in the proper submodule
		PyObject *pylast = PyList_GetItem(namelist, last);
		PyObject *dict = PyModule_Check(module) ? PyModule_GetDict(module)
		                                        : ClassObject_GetDict(module);
		PyDict_SetItem(dict, pylast, object.pointer());
		return PyReferenceBorrow<PyObject> (module);
	}

	PyReferenceBorrow<PyObject> insertIntoNamespace(const std::string& modulename,
							 PyObject *module, 
							 const std::string& cppname,
							 PyReferenceBorrow<PyObject> object)
	{
		// Split original C++ name by ::
		PyObject *pysplit = generatePythonModulePath(cppname);

		PyReferenceBorrow<PyObject> ret = insertIntoNamespace(modulename, module, pysplit, object);

		Py_XDECREF(pysplit);
		return ret;
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
		else if (type == "bool")     return (PyObject*)&PyBool_Type;
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
		else if (type == "short")			return c_short;
		else if (type == "unsigned short")			return c_ushort;
        else
			// not a primitive type
			throw LookupFailureException(type_name);
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
        } catch (LookupFailureException &) {
			try {
				return (PyObject*)fe.getTypeObject(templatearg);
			}
			catch (LookupFailureException &) {
				return NULL;
			}
        }
	}

	/**
	 * Runs a list of user-defined detectors, and if one succeeds, returns
	 * the result.
	 *
	 * @param type_list list of UserDefinedTranslator instances
	 * @param element the element to be detected
	 */
	Handle<RobinType> detectUserDefined
		(std::list<Handle<UserDefinedTranslator> > type_list, 
		 scripting_element element)
	{
		Handle<RobinType> user;
		typedef std::list<Handle<UserDefinedTranslator> > userdefinedtypelist;

		for (userdefinedtypelist::const_iterator userit = type_list.begin();
			 userit != type_list.end(); ++userit) {
			if (user = (*userit)->detectType(element))
				return user;
		}

		return Handle<RobinType>();
	}

	/* Integer bit sizes */
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

	Handle<Conversion> hlong2bool(new TrivialConversion);
	hlong2bool->setWeight(Conversion::Weight(0,0,0,1));
	hlong2bool       ->setSourceType(ArgumentLong);
	hlong2bool       ->setTargetType(ArgumentBoolean);
	ConversionTableSingleton::getInstance()->registerConversion(hlong2bool);

	Handle<Conversion> hbool2long(new TrivialConversion);
	hbool2long->setWeight(Conversion::Weight(0,0,0,1));
	hbool2long       ->setSourceType(ArgumentBoolean);
	PyReferenceSteal<PyLongObject> oneLong((PyLongObject*)PyLong_FromLong(1));
	Handle<RobinType> smallLongsType = BoundedNumericRobinType::giveTypeForNum(oneLong);
	hbool2long       ->setTargetType(smallLongsType);
	ConversionTableSingleton::getInstance()->registerConversion(hbool2long);

	Handle<Conversion> hdouble2float(new TrivialConversion); //No checks?
	hdouble2float    ->setSourceType(ArgumentDouble);
	hdouble2float    ->setTargetType(ArgumentFloat);
	ConversionTableSingleton::getInstance()->registerConversion(hdouble2float);

	Handle<Conversion> hlong2double(new IntToFloatConversion);
	hlong2double->setWeight(Conversion::Weight(0,2,0,0));
	hlong2double     ->setSourceType(ArgumentLong);
	hlong2double     ->setTargetType(ArgumentDouble);
	ConversionTableSingleton::getInstance()->registerConversion(hlong2double);

	Handle<Conversion> hchar2uchar(new TrivialConversion);
	hchar2uchar      ->setSourceType(ArgumentChar);
	hchar2uchar      ->setTargetType(ArgumentUChar);
	ConversionTableSingleton::getInstance()->registerConversion(hchar2uchar);

	Handle<Conversion> hchar2string(new TrivialConversion);
	hchar2string     ->setSourceType(ArgumentChar);
	hchar2string     ->setTargetType(ArgumentPascalString);
	ConversionTableSingleton::getInstance()->registerConversion(hchar2string);

	Handle<Conversion> hpascal2cstring(new PascalStringToCStringConversion);
	hpascal2cstring  ->setSourceType(ArgumentPascalString);
	hpascal2cstring  ->setTargetType(ArgumentCString);
	ConversionTableSingleton::getInstance()->registerConversion(hpascal2cstring);



}

Handle<RobinType> PythonFrontend::detectType_basic(PyObject *object) const
{
	PyObject *obtype = PyObject_Type(object);
	Handle<RobinType> user;

	if (PyFloat_Check(object))
		return ArgumentDouble;
	else if (PyString_Check(object))
		return (PyString_Size(object)==1)? ArgumentChar : ArgumentPascalString;
	else if (PyTuple_Check(object))
		return ArgumentPythonTuple;
	else if (ClassObject_Check(obtype)) {
		// - instance object
		Handle<Class> clas = ((ClassObject*)obtype)->getUnderlying();
		return clas->getType();
	}
	else if (PyObject_Type(obtype) == (PyObject*)EnumeratedTypeTypeObject) {
		// - enumerated constant object
		Handle<EnumeratedConstant> enumconst = 
			((EnumeratedConstantObject*)object)->getUnderlying();
		return enumconst->getType()->getType();
	}
	else if (user = detectUserDefined(m_userTypes, object)) {
		// - user-defined type object
		return user;
	}
	else if (PyInstance_Check(object) || _PyObject_GetDictPtr(object) != 0
			 || PyCObject_Check(object)) {
		// instance of a python class
		// class which has a dictionary
		// cobject
		return ArgumentScriptingElementNewRef;
	}
	else if (obtype == (PyObject*)&AddressTypeObject) {
		Handle<Address> address = ((AddressObject*)object)->getUnderlying();
		return address->getPointerType();
	}
	else
		return Handle<RobinType>();
}


Handle<RobinType> PythonFrontend::detectType_mostSpecific(scripting_element element)
	const
{

	PyObject *object = reinterpret_cast<PyObject*>(element);
	Handle<RobinType> user;

	if (object == Py_True || object == Py_False)
	{
		//this check has to be before checking if the type is an PyInt
		return ArgumentBoolean;
	} else if (object == Py_None) {
		return ArgumentVoid;
	}
	else if (PyInt_Check(object))
	{
		return BoundedNumericRobinType::giveTypeForNum(
				PyReferenceBorrow<PyIntObject>((PyIntObject*)object));
	}
	else if (PyLong_Check(object)) {
		return BoundedNumericRobinType::giveTypeForNum(
				PyReferenceBorrow<PyLongObject>((PyLongObject*)object));
	}
	else	 if((user = detectType_basic(object))) {
		return user;
	}
	else if (PyList_Check(object)) {
		Handle<RobinType> firstElementType;
		if(PyList_Size(object)!=0) {
			PyObject *firstElement = PyList_GET_ITEM(object,0);
			// Obtaining the type of the elements, but also
			// forcing list elements to be constant.
			// Currently robin does not care to update back the
			// elements of a list when its contents are modified,
			// only they are replaced in the list.
			firstElementType = PythonFrontend::detectType_mostSpecific(firstElement);
			if(firstElementType->isConstant() != RobinType::constReferenceKind)
			{

				firstElementType = ConstType::searchOrCreate(*firstElementType);
			}
		}
		//Returning a non-const list for the element type found
		return ListRobinType::listForSpecificElements(firstElementType);
	}
	else if (PyDict_Check(object)) {
		Handle<RobinType> firstKeyType;
		Handle<RobinType> firstValueType;

		PyObject *key, *value;
		Py_ssize_t pos = 0;
		if (PyDict_Next(object, &pos, &key, &value)) {
		    /* will bring a key-value pair to obtain
		     * their types.
		     * Of course only if the dictionary is not empty
		     */
			firstKeyType = PythonFrontend::detectType_mostSpecific(key);
			firstValueType = PythonFrontend::detectType_mostSpecific(value);
			if(firstKeyType->isConstant() != RobinType::constReferenceKind)
			{
				firstKeyType = ConstType::searchOrCreate(*firstKeyType);
			}
			if(firstValueType->isConstant() != RobinType::constReferenceKind)
			{
				firstValueType = ConstType::searchOrCreate(*firstValueType);
			}
		}
		return DictRobinType::dictForSpecificKeyAndValues(firstKeyType,firstValueType);

	}
	throw UnsupportedInterfaceException();
}

Handle<RobinType> PythonFrontend::detectType_asPython(PyObject *object)
	const
{
	/*
	 * TODO: when detectType is ready reimplement detectType_asPython
	 * using detectType
	 */
	Handle<RobinType> user;

	if (object == Py_True || object == Py_False)
	{
		//this check has to be before checking if the type is an PyInt
		return ArgumentBoolean;
	} else if (object == Py_None) {
		return ArgumentVoid;
	}
	else if (PyInt_Check(object))
	{
			return ArgumentLong;
	}
	else if (PyLong_Check(object))
	{
		return ArgumentPythonLong;
	}
	else if((user = detectType_basic(object))) {
		return user;
	}
	else if (PyList_Check(object)) {
		return ListRobinType::getGeneralListRobinType();
	}
	else if (PyDict_Check(object)) {
		return DictRobinType::getGeneralDictRobinType();
	}
	throw UnsupportedInterfaceException();
}


/**
 * Statically detect a type represented by a Python PyTypeObject.
 */
Handle<RobinType> PythonFrontend::detectType(PyTypeObject *pytype)
	const
{
	/*
	  This function should be able to translate from a python type
	  to a robin type representing it.
	  It should work exactly as detectType_asPython and detectType_asPython
	  should be based in this function.
	  TODO: Currently detectType is not divided between the types that can
	  be used from detectType_basic.
	  TODO: Currently there are a number of problems which prevent this function
	  to fully parse the python types.
	  These are the cases:

	  Enumerated types
	  ================
	  Notice cannot currently detect the type of an enumerated object
	  because all the enumerated types have the same type, which is not logical

	  User defined types (types registered from other modules)
	  ========================================================
	  Currently detectUserDefined and other related functions use
	  the element to know what is the robin type of the object, but
	  cannot detect it only by the PyTypeObject* element.
	  Need to redesign the mechanism.

	  AddressTypeObject (pointer types)
	  =================================
	  Those are pointer to types. The same problem as with user defined
	  types, still there is no support to know exactly the type of the object
	  without having the object itself, need to change this.

	  Unknown types
	  ==============
	  At first sight Types which do not belong to any other category should be considered
	  of type ArgumentScriptingElementNewRef.
	  The old code says otherwise; it only accepts three types of parameters
	  for ArgumentScriptingElementNewRef:
	    - types inheriting from a ClassTypeObject:
	        those are probably python classes which extend the ClassTypeObject
	    - types which do not have a instance dictionary:
	        those are simple types or builtin types, not python classes
	        (this definition is very problematic because we need the object to
	        detect if it has a dictionary using the function _PyObject_GetDictPtr)
	    - types exported from other extension modules written in C/C++
	       (CObjects)
	   In my opinion all of the aforementioned cases cover pretty much of
	   the types that robin will need to handle and are not of any other category.
	   Because of that, currently i simply return ArgumentScriptingElementNewRef
	   as a default type if no other type matches.


	}*/


	//FASTCHECKSUBTYPE first checks equality of the types
	// then uses the normal subtypes mechanism
	// It is copied from pythons own
#    define FASTCHECKSUBTYPE(subtype,supertype) \
	((subtype) == (supertype) || PyType_IsSubtype((subtype), (supertype)))

	if (FASTCHECKSUBTYPE(pytype,&PyBool_Type))
	{
			//this check has to be before checking if the type is an PyInt
			return ArgumentBoolean;
	}
	else if (FASTCHECKSUBTYPE(pytype,&PyInt_Type))
		return ArgumentLong;
	else if (FASTCHECKSUBTYPE(pytype,&PyLong_Type))
		return ArgumentPythonLong;
	else if (FASTCHECKSUBTYPE(pytype,&PyList_Type))
		return ListRobinType::getGeneralListRobinType();
	else if (FASTCHECKSUBTYPE(pytype,&PyFloat_Type))
		return ArgumentDouble;
	else if (FASTCHECKSUBTYPE(pytype,&PyString_Type))
		return ArgumentPascalString;
	else if (FASTCHECKSUBTYPE(pytype,&PyTuple_Type))
		return ArgumentPythonTuple;
	else if (FASTCHECKSUBTYPE(pytype,&PyDict_Type))
		return DictRobinType::getGeneralDictRobinType();
	else if (pytype->ob_type == ClassTypeObject) {
		// Only ClassType enters here.
		// because if there is a python class inheriting from our type
		// we need to take care about it differently.
		Handle<Class> clas = ((ClassObject*)pytype)->getUnderlying();
		return clas->getType();
	}
	else {
		return ArgumentScriptingElementNewRef;
	}

#undef FASTCHECKSUBTYPE
}


/**
 * Creates a Python adapter for a specific type.
 */
Handle<Adapter> PythonFrontend::giveAdapterFor(const RobinType& type)
	const
{
	Type basetype = type.basetype();

	if (dynamic_cast<const PointerType*>(&type)) {
		const PointerType*pointerType = dynamic_cast<const PointerType*>(&type);
		Handle<RobinType> pointedType = pointerType->pointed().get_handler();
		return Handle<Adapter>(new AddressAdapter(pointedType));
	}
	if (basetype.category == TYPE_CATEGORY_INTRINSIC) {
		typedef PySignedNumTraits<short> t_short;
		typedef PySignedNumTraits<int> t_int;
		typedef PySignedNumTraits<long> t_long;
		typedef PySignedNumTraits<long long> t_longlong;
		typedef PyUnsignedNumTraits<unsigned short> t_ushort;
		typedef PyUnsignedNumTraits<unsigned int> t_uint;
		typedef PyUnsignedNumTraits<unsigned long> t_ulong;
		typedef PyUnsignedNumTraits<unsigned long long> t_ulonglong;
		if (basetype.spec == TYPE_INTRINSIC_INT)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<int,  t_int>());
		else if (basetype.spec == TYPE_INTRINSIC_UINT)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<unsigned int, t_uint>());
		else if (basetype.spec == TYPE_INTRINSIC_LONG)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<long, t_long>());
		else if (basetype.spec == TYPE_INTRINSIC_LONG_LONG)
			return Handle<Adapter>
				(new AllocatedPrimitivePythonAdapter<long long, t_longlong>());
		else if (basetype.spec == TYPE_INTRINSIC_ULONG)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<unsigned long, t_ulong>());
		else if (basetype.spec == TYPE_INTRINSIC_ULONG_LONG)
			return Handle<Adapter>
				(new AllocatedPrimitivePythonAdapter<unsigned long long,
				                                     t_ulonglong>());
		else if (basetype.spec == TYPE_INTRINSIC_SHORT)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<short, t_short>());
		else if (basetype.spec == TYPE_INTRINSIC_USHORT)
			return Handle<Adapter>
				(new SmallPrimitivePythonAdapter<unsigned short, t_ushort>());
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
				(new SmallPrimitivePythonAdapter<char *, PyStringTraits>
				 ());
		else if (basetype.spec == TYPE_EXTENDED_PASCALSTRING)
			return Handle<Adapter>(new PascalStringAdapter);
		else if (basetype.spec == TYPE_EXTENDED_ELEMENT) {

            if(type.isBorrowed()) {
    			return Handle<Adapter>(new BorrowedAdapter<PyObjectAdapter>);
            } else {
                return Handle<Adapter>(new PyObjectAdapter);
            }
        }
		else
			throw UnsupportedInterfaceException();
	}
	else if (basetype.category == TYPE_CATEGORY_USERDEFINED) {
		if (basetype.spec == TYPE_USERDEFINED_OBJECT) {
			// - find the class object
			ClassObject *classobj = getClassObject(basetype.objclass);
			// - create instance adapter
			bool owned = (&type == &*(basetype.objclass->getPtrType()));
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
	PyObject *module = Py_InitModule(const_cast<char*>(newcomer.name().c_str()), meth);

	Handle<Namespace> globals = newcomer.globalNamespace();

	typedef Handle<Namespace::NameIterator> hiterator;
	// Expose classes
	for (hiterator cname_iter = globals->enumerateClasses();
		 !cname_iter->done(); cname_iter->next()) {
		// Create a class object in the module
		std::string name = cname_iter->value();


		Handle<Class> clas = globals->lookupClass(name);
		ClassObject *pyclass = getClassObject(clas);
		PyReferenceBorrow<PyObject> mod = insertIntoNamespace(newcomer.name(), module, name, PyReferenceBorrow<PyObject>((PyObject*)pyclass));
		pyclass->inModule(mod.pointer());

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
			Handle<ActualArgumentList> noargs(new ActualArgumentList);
            KeywordArgumentMap nokwargs;
			PyReferenceSteal<PyObject> pyvar ( (PyObject*)routine->call(noargs, nokwargs) );
			insertIntoNamespace(newcomer.name(), module, varname, pyvar);
		}
		else {
			// Create a function object in the module
			PyReferenceSteal<FunctionObject> pyroutine = FunctionObject::construct(routine);
			pyroutine->setName(name);
			pyroutine->inModule(module);
			insertIntoNamespace(newcomer.name(), module, name, static_pyhcast<PyObject>(pyroutine));
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
							name, PyReferenceBorrow<PyObject>((PyObject*)pyenumtype));
		// Get the list of constants belonging to this type
		std::vector<Handle<EnumeratedConstant> > enumconstants =
			listOfConstants(enumtype);
		// Create objects representing enum constants
		for (std::vector<Handle<EnumeratedConstant> >::iterator ci =
				 enumconstants.begin(); ci != enumconstants.end(); ++ci) {
			PyReferenceSteal<PyObject> obj(new EnumeratedConstantObject(pyenumtype, *ci));
			PyObject *namespaces = generatePythonModulePath(name);
			PyList_SetItem(namespaces,
			               PyList_Size(namespaces) - 1,
			               PyString_FromString((*ci)->getLiteral().c_str()));
			insertIntoNamespace(newcomer.name(), module, 
								namespaces, PyReferenceBorrow<PyObject>(obj));
		}
	}
	// Apply aliases
	for (hiterator alias_iter = globals->enumerateAliases();
		 !alias_iter->done(); alias_iter->next()) {
		try {
			std::string name = alias_iter->value();
            Handle<Class> aliased = RegistrationMechanismSingleton::getInstance()->findClass(name);
			ClassObject *pyaliased = m_classes[&*aliased];
			insertIntoNamespace(newcomer.name(), module, name,
								PyReferenceBorrow<PyObject>((PyObject*)pyaliased));
			// Register class in classes map by its alias as well
			m_classesByName[name] = pyaliased;
		}
		catch (const LookupFailureException& ) {
            // this could be a primitive type
            try {
				std::string name = alias_iter->value();
				std::string unaliasedName = name;
				RegistrationMechanismSingleton::getInstance()->unalias(unaliasedName);
				PyObject *prim_type = getPrimitiveType(globals, unaliasedName);
				insertIntoNamespace(newcomer.name(), module, name,
						PyReferenceBorrow<PyObject>(prim_type));
				m_typesByName[name] = (PyTypeObject*)prim_type;
				continue;
            } catch(const LookupFailureException&) {
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
					PyReferenceBorrow<PyObject>((PyObject*)pytemplate));
	}
	for (hiterator tfname_iter = globals->enumerateFunctions();
		 !tfname_iter->done(); tfname_iter->next()) {
		// Create or access a template object if necessary
		std::string funcname = tfname_iter->value(), templatename;
		TemplateObject *pytemplate = exposeTemplate(globals, T_FUNCTION, funcname,
													templatename);
		if (pytemplate)
			insertIntoNamespace(newcomer.name(), module, templatename, 
					PyReferenceBorrow<PyObject>((PyObject*)pytemplate));
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
	size_t lt = classname.find('<');

	// if it wasn't found, this is not a template
	if (lt == classname.npos || strncmp(classname.c_str(), "operator", strlen("operator")) == 0) {
		return false;
	}
	else {
		// find the last > that closes this one
		size_t gt = classname.rfind('>');
		// if there is no final bracket (this cannot possibly happen)
		assert_true(gt < classname.length());
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
		size_t parenthesisCount;
		while (templatearg.size()) {
			// find the next comma
			size_t i;
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
		// If there is more than one argument, use a tuple
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
			PyObject *regobj = getRegisteredObject(kind, elemname);
			// Update the dictionary with the current element
			if (regobj != NULL)
				PyObject_SetItem((PyObject*)pytemplate,
								 pyarguments, regobj);
			Py_XDECREF(pyarguments);
		}

		return pytemplate;
	}
	else {
		return NULL;
	}
}

PyObject *PythonFrontend::getRegisteredObject(TemplateKind kind,
										  const std::string& name)
{
	switch (kind) {
	case T_CLASS:    return (PyObject *)(PyTypeObject*)getClassObject(name);
	case T_FUNCTION: return getFunctionObject(name).pointer();
	default:         return NULL;
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

void PythonFrontend::own(scripting_element master, scripting_element slave)
{
	if (!InstanceObject_Check((PyObject*)master)) {
		return ;
	}
	
    InstanceObject *imaster = reinterpret_cast<InstanceObject*>(master);

    imaster->keepAlive((PyObject*)slave);
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
 * Finds a corresponding Python type object for a C++ type.
 *
 * @param name fully-qualified type name
 */
PyTypeObject *PythonFrontend::getTypeObject(const std::string& name) const
{
	typenameassoc::const_iterator typefind = m_typesByName.find(name);
	if (typefind == m_typesByName.end()) {
		ClassObject *klass = getClassObject(name);
		if (klass == NULL) {
			throw LookupFailureException(name);
		}
		else {
			return (PyTypeObject*)klass;
		}
	}
	else {
		assert_true(typefind->second != NULL);
		return typefind->second;
	}
}

/**
 * Finds a Python::ClassObject via a Robin::Class reference.
 * Additionaly it adds it to m_classes and m_classesByName
 */
ClassObject *PythonFrontend::getClassObject(Handle<Robin::Class> clas) const
{
	classassoc::const_iterator classfind = 
		m_classes.find(&*clas);
	// If class exists in map, return associated value.
	// Otherwise, create a new class object and register it.
	if (classfind == m_classes.end()) {
		ClassObject *newClassObject = new ClassObject(clas);
		m_classesByName[clas->name()] = newClassObject;
		m_classes[&*clas] = newClassObject;
		return newClassObject;
	}
	else {
		assert_true(classfind->second != NULL);
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
		assert_true(classfind->second != NULL);
		return classfind->second;
	}
}

/**
 * Finds a Python::FunctionObject by literal name.
 * Returns <b>NULL</b> if none exist.
 */
PyReferenceBorrow<FunctionObject> PythonFrontend::getFunctionObject(const std::string& name)
	const
{
	funcnameassoc::const_iterator funcfind = m_functionsByName.find(name);
	// If class exists in map, return associated value.
	if (funcfind == m_functionsByName.end()) {
		return PyReferenceBorrow<FunctionObject>();
	}
	else {
		assert_true(funcfind->second);
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
		assert_true(enumfind->second != NULL);
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
		assert_true(templfind->second != NULL);
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
			Py_ssize_t nitems = PyList_Size(items);
			assert_true(nitems >= 0);
			for (Py_ssize_t i = 0; i < nitems; ++i) {
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
	: m_type(RobinType::create_new(TYPE_CATEGORY_USERDEFINED,
					TYPE_USERDEFINED_INTERCEPTOR,pytype->tp_name,RobinType::regularKind)),
	  m_pytype(pytype)
{
}

ByTypeTranslator::~ByTypeTranslator()
{

}

Handle<RobinType> ByTypeTranslator::detectType(scripting_element element)
{
	PyObject *pyelement = (PyObject*)element;
	if (PyObject_Type(pyelement) == (PyObject*)m_pytype) {
		return m_type;
	}
	else {
		return Handle<RobinType>();
	}
}



} // end of namespace Robin::Python

} // end of namespace Robin

