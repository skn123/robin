/* -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file
 *
 * @par SOURCE
 * registration/mechanism.cc
 *
 * @par TITLE
 * Registration Mechanism Control Center
 *
 * @par PACKAGE
 * Robin
 *
 * @par PUBLIC-CLASSES
 *
 * <ul><li>RegistrationMechanism</li>
 * <li>RegistrationMechanismSingleton</li></ul>
 */

#include "mechanism.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <errno.h>
#include <robin/debug/assert.h>

#include <robin/reflection/library.h>
#include <robin/reflection/class.h>
#include <robin/reflection/pointer.h>

#include "../reflection/cfunction.h"
#include "../reflection/overloadedset.h"
#include "../reflection/intrinsic_type_arguments.h"
#include "../reflection/enumeratedtype.h"
#include "../reflection/conversiontable.h"
#include "../reflection/fundamental_conversions.h"
#include "../reflection/interface.h"

#include "../frontends/framework.h"

#include "../debug/trace.h"


namespace Robin {



/**
 * Builds and initializes the registration mechanism.
 * Don't invoke this constructor - it must be called once using the
 * singleton object, <classref>RegistrationMechanismSingleton</classref>.
 */
RegistrationMechanism::RegistrationMechanism()
	: m_ns_common("<common>")
{
}

/**
 * Retrieve the registration data stored in the library
 * in the form of <classref>RegData</classref> compatible structures.
 * The library should and will be dynamically opened using
 * <b>dlopen</b>.
 */
RegData *RegistrationMechanism::acquireRegData(std::string library)
{
	try {
		return acquireRegData_impl(library);
	}
	catch (DynamicLibraryOpenException& e) {
		dbg::traceRegistration << "// " << library << ": " << e.dlerror_at
				   << dbg::endl;
		throw e;
	}
}

/**
 * Opens a library and builds a corresponding
 * <classref>Library</classref> object, registering all the components
 * of the library in the process. These can then be accessed through
 * the library's global namespace.
 */
Handle<Library> RegistrationMechanism::import(const std::string& library,
											  const std::string& name)
{
	Handle<Library> lib(new Library(name));
	// Get library's registration information
	RegData *reg_entry = acquireRegData(library);
	// Fill library with information
	admit(reg_entry, Handle<Class>(), *(lib->globalNamespace()));
	return lib;
}
Handle<Class> RegistrationMechanism::findClass(const std::string &name)
{
	return m_ns_common.lookupClass(name);
}

void RegistrationMechanism::unalias(std::string &name)
{
	return m_ns_common.unalias(name);
}


/**
 * Opens a library and builds a corresponding
 * <classref>Library</classref> object, registering all the components
 * of the library in the process. These can then be accessed through
 * the library's global namespace.
 */
Handle<Library> RegistrationMechanism::import(const std::string& library)
{
	return import(library, library);
}

/**
 * Unpacks data stored in <classref>RegData</classref>
 * structure, and generates Robin reflection elements that correspond
 * to those described by these structures.
 */
void RegistrationMechanism::admit(RegData *rbase, Handle<Class> klass,
								  Namespace& container)
{
	dbg::traceRegistration <<  "admit." << container << dbg::endl;
	if (!rbase) return;
	if (klass && !klass->isEmpty()) return;  /* avoid double loading */

	for (RegData *pdata = rbase;
		 pdata->name != 0 /* null indicates end of list */; ++pdata) {

		normalizeName(pdata);

		if (strcmp(pdata->type, "enum") == 0) {             /* type = enum */
			Handle<EnumeratedType> ne = touchEnum(pdata->name, container);
			admitEnum(pdata->prototype, ne);
		}
		else if (strcmp(pdata->type, "class") == 0) {      /* type = class */
			// recursively admit
			Handle<Class> subclass = touchClass(pdata->name, m_ns_common);
			container.declare(pdata->name, subclass);
			dbg::traceClassRegistration << "Registering class " << pdata->name << dbg::endl;
			admit(pdata->prototype, subclass, container);
		}
		else if (strcmp(pdata->type, "extends") == 0) {     /* type = extend */
			// add inheritance
			Handle<Class> baseclass = touchClass(pdata->name, m_ns_common);
			if (klass) {
				klass->inherit(baseclass);
				// Register up-cast conversion
				symbol sym = pdata->sym;
				if (sym != 0)
					admitUpCastConversion(klass, baseclass, sym);
			}
		}
		else if (strcmp(pdata->type, "constructor") == 0) { /* type = ctor */
			dbg::traceRegistration << "registering constructor for type" << klass->name() <<dbg::endl;
			Handle<CFunction> cfun(new CFunction((symbol)pdata->sym,klass->name(),Robin::CFunction::Constructor,klass->name()));
			admitArguments(pdata->prototype, cfun, container);
			char symbol = pdata->name[0];
			// symbol indicates the conversion type implied by this
			// constructor:
			//   '%' = no conversion ("explicit")
			//   '*' = user-defined conversion
			//   '^' = promotion
			if (cfun->signature().size() == 1 && symbol != '%')
				admitConstructorConversion(cfun, klass,
										   symbol == '^');
			assert_true(klass);
			klass->addConstructor(cfun);
			dbg::traceRegistration << "finish registering constructor" << dbg::endl;
		}
		else if (strcmp(pdata->type, "destructor") == 0) { /* type = dtor */
			Handle<CFunction> cfun(new CFunction((symbol)pdata->sym,std::string("~")+klass->name(),Robin::CFunction::Destructor,klass->name()));
			if (klass) {
				cfun->addFormalArgument(klass->getPtrType());
				klass->setDestructor(cfun);
			}
		}
		else if (pdata->type[0] == '=') {               /* type = alias */
			std::string actual = pdata->type + 1, aliased = pdata->name;
			// - a special case is a "typedef struct A A;" declaration
			if (actual == aliased) {
				Handle<Class> self_struct = touchClass(actual, m_ns_common);
				container.declare(actual, self_struct);
			}
			else {
				dbg::traceClassRegistration << "Registering alias " << pdata->name << dbg::endl;
				container.alias(pdata->type + 1, pdata->name);
				m_ns_common.alias(pdata->type + 1, pdata->name);
			}
		}
		else /* assume it's a function */ {             /* type = function */
			dbg::traceRegistration << "Registering function " << pdata->name <<dbg::endl;
			symbol sym = (symbol)pdata->sym;
			if (sym != 0) {

				CFunction::FunctionKind kind;
				std::string klassName;
				if(klass)
				{
					kind = CFunction::Method;
					klassName = klass->name();
				} else {
					kind = CFunction::GlobalFunction;
				}

				// Build a CFunction with the symbol as a flat wrapper
				Handle<CFunction> cfun(new CFunction(sym,pdata->name,kind,klassName));
				// set the return type
				if (strcmp(pdata->type, "void") != 0) {
					Handle<RobinType> ret =
						interpretType(pdata->type, container);
					cfun->specifyReturnType(ret);
					if (ret->isReference())
						cfun->supplyMemoryManagementHint(false);
				}
				// set arguments
				if (klass) {
					// - instance method
					cfun->addFormalArgument(klass->getType());
				}
				admitArguments(pdata->prototype, cfun, container);
				// add function to namespace or class
				if (klass) {
					if (pdata->name[0] == '!')
					{
						cfun->m_allow_edge = false;
						klass->addInstanceMethod(pdata->name+1, cfun);
					}
					else
					{
						cfun->m_allow_edge = true;
						klass->addInstanceMethod(pdata->name, cfun);
					}
				}
				else {
					Handle<OverloadedSet> ols;
					// See if any overloaded set exists with this name
					try {
						ols = static_hcast<OverloadedSet>(container.lookupFunction(pdata->name));
					}
					catch (LookupFailureException& ) {
						// No? create a new one.
						ols = OverloadedSet::create_new(pdata->name);
						container.declare(pdata->name, static_hcast<Callable>(ols));
					}
					// now add cfun
					dbg::traceRegistration << "// @FUNC: " << pdata->name << " with "
							   << int(cfun->signature().size())
							   << " arguments." << dbg::endl;
					ols->addAlternative(cfun);
				}
			}
			else {
				// A pure virtual function (sym = 0)
				Handle<Signature> signature( new Signature);
				signature->name = pdata->name;
				if (strcmp(pdata->type, "void") != 0)
					signature->returnType = interpretType(pdata->type, container);
				else
					signature->returnType = Handle<RobinType>();
				admitArguments(pdata->prototype, *signature, container);
				pdata->sym = signature.release();
			}
		}
	}
}

/**
 * Registers the formal arguments of a function.
 */
void RegistrationMechanism::admitArguments(const RegData *rbase,
										   Handle<CFunction> cfun,
										   Namespace &container)
{
	dbg::traceRegistration << "Admitting parameters" <<dbg::endl;
	dbg::IndentationGuard grd(dbg::traceRegistration);
	if (!rbase) return;

	for (const RegData *pdata = rbase;
		 pdata->name != 0 /* null indicates end of list */; ++pdata) {
		// Create an argument of this type
		Handle<RobinType> argtype = interpretType(pdata->type, container);
		dbg::traceRegistration << "adding parameter" << *argtype << dbg::endl;
		cfun->addFormalArgument(pdata->name, argtype);
	}
}

/**
 * Constructs a signature from a formal declaration contained in the library.
 */
void RegistrationMechanism::admitArguments(const RegData *rbase,
										   Signature& signature,
										   Namespace& container)
{
	if (!rbase) return;

	for (const RegData *pdata = rbase;
		 pdata->name != 0 /* null indicates end of list */; ++pdata) {
		// Create an argument of this type
		Handle<RobinType> argtype = interpretType(pdata->type, container);
		signature.argumentTypes.push_back(argtype);
	}
}

/**
 * Registers the constant elements of an enumerated
 * type.
 */
void RegistrationMechanism::admitEnum(const RegData *rbase,
									  Handle<EnumeratedType> etype)
{
	if (!rbase) return;

	for (const RegData *pdata = rbase;
		 pdata->name != 0 /* null indicates end of list */; ++pdata) {
		// Refer to current item as constant definition (name and value)
		const int *pvalue = reinterpret_cast<const int*>(pdata->sym);
		etype->addConstant(pdata->name, *pvalue);
	}
}

/**
 * Service; returns a handle to a <classref>RobinType
 * </classref> which denotes the type that the string indicates:
 * <ul>
 *  <li><tt>int</tt>,<tt>float</tt>, etc. - the corresponding intrinsic
 *    type.</li>
 *  <li><i>enum-name</i> - enumerated type with the specified name.</li>
 *  <li><tt>&amp;</tt><i>class-name</i> - reference to class instance.</li>
 *  <li><tt>*</tt><i>class-name</i> - pointer to class instance.</li>
 * </ul>
 */
Handle<RobinType> RegistrationMechanism::interpretType
    (const char *type, Namespace& container)
{
	struct entry { const char *keyword; Handle<RobinType> toa; }
	intrinsics[] =
		{ { "int", ArgumentInt },
		  { "unsigned int", ArgumentUInt },
		  { "long", ArgumentLong },
		  { "unsigned long", ArgumentULong },
		  { "long long", ArgumentLongLong },
		  { "unsigned long long", ArgumentULongLong },
		  { "short", ArgumentShort },
		  { "unsigned short", ArgumentUShort },
		  { "float", ArgumentFloat },
		  { "double", ArgumentDouble },
		  { "&double", ArgumentDouble },
		  { "char", ArgumentChar },
		  { "unsigned char", ArgumentUChar },
		  { "signed char", ArgumentSChar },
		  { "bool", ArgumentBoolean },
		  { "void", ArgumentVoid },
		  { "*char", ArgumentCString },
		  { "@string", ArgumentPascalString },
		  { "scripting_element", ArgumentScriptingElementNewRef },
          { "&scripting_element", ArgumentScriptingElementBorrowedRef },
		  { "#scripting_element", ArgumentScriptingElementNewRef }, // @@@
		  { 0, Handle<Robin::RobinType>(0) } };

	Handle<RobinType> rtype;
	enum { ARG, PTRTYPE, CONSTTYPE, REGULARTYPE } modif = ARG;
	int redir_count = 0;

	while (!rtype) {
		// Is an intrinsic type?
		for (entry *ei = intrinsics; ei->keyword != 0; ++ei)
			if (strcmp(ei->keyword, type) == 0) rtype = ei->toa;

		// Enumerated type?
		if (!rtype && type[0] == '#')
			rtype = touchEnum(type + 1, container)->getType();

		if (!rtype) {
			// Reference or pointer?
			if (type[0] == '*') { modif = PTRTYPE; redir_count++; }
			else if (type[0] == '&') { modif = CONSTTYPE;  }
			else if (type[0] == '>') modif = REGULARTYPE;
			else break;

			type++;  /* skip ref/ptr symbol */
		}
	}

	if (!rtype) {
		// Class reference or pointer
		std::string typeString(type);
		Handle<Class> klass = touchClass(typeString, m_ns_common);

		switch (modif) {
		case ARG:    throw LookupFailureException(typeString);
		case PTRTYPE: rtype = klass->getPtrType(); --redir_count;      break;
		case CONSTTYPE: rtype = klass->getConstType();      break;
		case REGULARTYPE: rtype = klass->getType();                   break;
		default:
			assert_true(false);
		};
	}

	while (redir_count-- > 0) {
		FrontendsFramework::fillAdapter(rtype);
		rtype =  rtype->pointer();
	}

	FrontendsFramework::fillAdapter(rtype);

	return rtype;
}

/**
 * Registers a trivial conversion between arbitrary source and destination
 * types.
 */
void RegistrationMechanism::admitTrivialConversion(Handle<RobinType> from,
												   Handle<RobinType> to)
{
	Handle<Conversion> trivial(new TrivialConversion);
	trivial->setSourceType(from);
	trivial->setTargetType(to);
	ConversionTableSingleton::getInstance()->registerConversion(trivial);
}

/**
 * Allows constructors with one argument to become
 * conversions.
 */
void RegistrationMechanism::admitConstructorConversion(Handle<CFunction> ctor,
													   Handle<Class> klass,
													   bool promotion)
{
	Handle<Conversion> udc(new ConversionViaConstruction);
	udc->setSourceType(ctor->signature()[0]);
	udc->setTargetType(klass->getConstType());
	if (promotion)
		udc->setWeight(Conversion::Weight(0,1,0,0));
	ConversionTableSingleton::getInstance()->registerConversion(udc);
}


/**
 * Allows a conversion from child to parent in the
 * inheritance tree. The transformsym is a pointer to a function of
 * the form <code>Base *f(Derived *)</code> which actually performs
 * the cast, and this function is registered as the transform function
 * of the upcast.
 */
void RegistrationMechanism::admitUpCastConversion(Handle<Class> derived,
												  Handle<Class> base,
												  void *transformsym)
{
	UpCastConversion::transformfunc upfunc =
		(UpCastConversion::transformfunc)transformsym;

	// Construct conversion object with given transform func
	Handle<Conversion> upcast(new UpCastConversion(upfunc));
	upcast->setSourceType(derived->getType());
	upcast->setTargetType(base->getType());
	ConversionTableSingleton::getInstance()->registerConversion(upcast);

	// Construct conversion for the const type
	Handle<Conversion> upcastConst(new UpCastConversion(upfunc));
	upcastConst->setSourceType(derived->getConstType());
	upcastConst->setTargetType(base->getConstType());
	ConversionTableSingleton::getInstance()->registerConversion(upcastConst);

}

/**
 * Returns the class that goes by the name specified, if
 * one is declared in the container namespace, or creates an empty
 * new class if none is.
 */
Handle<Class> RegistrationMechanism::touchClass(const std::string& name,
												Namespace& container)
{
	Handle<Class> klass;

	try {
		klass = container.lookupClass(name);
	}
	catch (LookupFailureException& e) {
		// Create a new class with that name
		klass = Class::create_new(name);
		FrontendsFramework::fillAdapters(klass);
		container.declare(e.look, klass);
		// Add a trivial conversion from regular type to pointer type
		admitTrivialConversion(klass->getType(), klass->getPtrType());
		// Add a trivial conversion from ref to out
		admitTrivialConversion(klass->getType(), klass->getConstType());
	}

	return klass;
}

/**
 * Returns the enumerated type that goes by the name
 * specified, if one is declared in the container namespace, or creates
 * an empty new <classref>EnumeratedType</classref> if none is.
 */
Handle<EnumeratedType> RegistrationMechanism::touchEnum(const std::string&
														name,
														Namespace& container)
{
	Handle<EnumeratedType> kenum;

	try {
		kenum = container.lookupEnum(name);
	}
	catch (LookupFailureException& e) {
		// Create a new class with that name
		kenum = Handle<EnumeratedType>(new EnumeratedType(name));
		kenum->activate(kenum);
		FrontendsFramework::fillAdapter(kenum->getType());
		container.declare(e.look, kenum);
	}

	return kenum;
}

void RegistrationMechanism::normalizeName(RegData *reg) const
{
	// @deprecated this is a hack for backward compatibility and should be
	// removed in version 1.1.
	if (strcmp(reg->name, "operator *") == 0) reg->name = "operator*";
	if (strcmp(reg->name, "operator &") == 0) reg->name = "operator&";
}


#if defined(_WIN32)
#include <windows.h>

/**
 * (Win32 implementation)
 * Retrieve the registration data stored in the library
 * in the form of <classref>RegData</classref> compatible structures.
 * The library should and will be dynamically opened using
 * <b>dlopen</b>.
 */
RegData *RegistrationMechanism::acquireRegData_impl(std::string library)
{
	try {
		// LoadLibrary
		HMODULE libhandle = LoadLibrary(library.c_str());
		if (libhandle == NULL) throw DynamicLibraryOpenException();
		// fetch entry symbol
		void *entry = GetProcAddress(libhandle, "entry");
		if (entry == 0) throw DynamicLibraryOpenException();
		return reinterpret_cast<RegData*>(entry);
	}
	catch (DynamicLibraryOpenException& e) {
		LPVOID lpMsgBuf;
		if (FormatMessage(
		    FORMAT_MESSAGE_ALLOCATE_BUFFER |
		    FORMAT_MESSAGE_FROM_SYSTEM |
		    FORMAT_MESSAGE_IGNORE_INSERTS,
		    NULL,
		    GetLastError(),
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &lpMsgBuf,
		    0,
		    NULL ) != 0)
		{
			e.dlerror_at = (LPCTSTR) lpMsgBuf;
		}
		else {
			e.dlerror_at = "dynamic library error (unrecognized)";
		}
		throw e;
	}
}
#endif

#if defined(__unix) || defined(__APPLE__)
/**
 * (Unix implementation)
 * Retrieve the registration data stored in the library
 * in the form of <classref>RegData</classref> compatible structures.
 * The library should and will be dynamically opened using
 * <b>dlopen</b>.
 */
RegData *RegistrationMechanism::acquireRegData_impl(std::string library)
{
	try {
		// dlopen lib
		void *libhandle = dlopen(library.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (libhandle == 0) throw DynamicLibraryOpenException(errno);
		// fetch callback symbol
		Interface::callback_t *__callback =
			(Interface::callback_t*)dlsym(libhandle, "__robin_callback");
		if (__callback)
			*__callback = &Interface::global_callback;
		// fetch entry symbol
		void *entry = dlsym(libhandle, "entry");
		if (entry == 0) throw DynamicLibraryOpenException(errno);
		return reinterpret_cast<RegData*>(entry);
	}
	catch (DynamicLibraryOpenException& e) {
		e.dlerror_at = dlerror();
		throw e;
	}
}
#endif


/**
 */
DynamicLibraryOpenException::DynamicLibraryOpenException()
	: errno_at(errno)
{ }

/**
 */
DynamicLibraryOpenException::DynamicLibraryOpenException(int merr)
	: errno_at(merr)
{ }

/**
 */
const char *DynamicLibraryOpenException::what() const throw()
{
	return "invalid or malformed module library.";
}

namespace {
	/**
	 * A dummy class.
	 * When the library is unloaded, deletes the instance of the
	 * registration mechanism.
	 * NOTE: might be there is a way of doing this in Pattern::Singelton,
	 *      but need to explore the consequences.
	 */
	class RegistrationMechanismDeallocator
	{
	public:
		~RegistrationMechanismDeallocator() {
			delete RegistrationMechanismSingleton::getInstance();
		}
	} _rmdealloc;
}


} // end of namespace Robin
