/* -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @par SOURCE
 * extreme/interactive/syntax.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "syntax.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iostream>

// Robin includes
#include <robin/reflection/callable.h>
#include <robin/reflection/cfunction.h>
#include <robin/reflection/namespace.h>
#include <robin/reflection/class.h>
#include <robin/reflection/instance.h>
#include <robin/reflection/library.h>
#include <robin/reflection/intrinsic_type_arguments.h>

#include <robin/frontends/simple/elements.h>
#include <robin/frontends/simple/instanceelement.h>

#include <robin/registration/mechanism.h>


#include <robin/debug/trace.h>

extern "C" void yylex();
extern "C" void yyrestart(FILE *);
extern FILE *yyin;

#define STORAGE_CELLS 6

namespace {


	Robin::scripting_element wrapInstance(Handle<Robin::Instance> instance)
	{
		Simple::Element *wrapped =
			new Robin::SimpleInstanceObjectElement(instance);
		return (Robin::scripting_element)wrapped;
	}

	Handle<Robin::EnumeratedType> lookupEnumIn(const Robin::Namespace& ns,
												 const std::string& name)
	{
		return ns.lookupEnum(name);
	}

	Handle<Robin::Class> lookupClassIn(const Robin::Namespace& ns,
										 const std::string& name)
	{
		return ns.lookupClass(name);
	}

	Handle<Robin::Callable> lookupRoutineIn(const Robin::Namespace& ns,
											  const std::string& name)
	{
		return ns.lookupFunction(name);
	}

	template < class ELEM, class LOOK, class ITER >
	Handle<ELEM> multipleLookup(ITER begin, ITER end, LOOK lookUp, 
								const std::string& name)
	{
		for (ITER iter = begin; iter != end; ++iter) {
			try {
				return lookUp(**iter, name);
			}
			catch (Robin::LookupFailureException& e) {
				// continue
			}
		}
		throw Robin::LookupFailureException(name);
	}

	const std::string libdir = "lib/linux/";
}

std::ostream& operator<<(std::ostream& os, const Simple::Element& e)
{
	e.dbgout();
	return os;
}


/**
 */
void InteractiveSyntaxAnalyzer::string(const char *phrase)
{
	std::string value(phrase + 1, strlen(phrase) - 2); // cut off the quotes
	literal(value);
}

/**
 */
void InteractiveSyntaxAnalyzer::blank()
{
	reuse();
}

/**
 */
void InteractiveSyntaxAnalyzer::variable(const char *name)
{
	reuse(var_lookup(name));
}

/**
 */
void InteractiveSyntaxAnalyzer::address(const char *name)
{
	reuse_address(var_lookup(name));
}

/**
 */
void InteractiveSyntaxAnalyzer::dereference(const char *name)
{
	reuse_by_address(var_lookup(name));
}

/**
 */
void InteractiveSyntaxAnalyzer::assign(const char *name)
{
	var(var_lookup(name));
}

int InteractiveSyntaxAnalyzer::var_lookup(const char *name)
{
	return name[0] - 'A';
}


class NotAnInstanceException : public std::exception
{
public:
	const char *what() const throw() 
	{ return "instance required for method invocation."; }
};



typedef std::vector<Simple::Element*> PersonalArgumentList;

struct InteractiveSyntaxAnalyzer::Imp 
{
	enum Activity { ACT_NOTHING, ACT_FUNCALL, ACT_IMPORT } activity;

	bool args_mode;
	char *function;
	Robin::ActualArgumentList args;
	PersonalArgumentList personal_args;
	Simple::Element *personal_instance;
	int destination;

	std::vector<Handle<Robin::Namespace> > globals;
	Handle<Robin::EnumeratedType> locals;
	Simple::Element *cells[STORAGE_CELLS];   /* these are like variables */

	Simple::Element * _;  /* result of last non-void call 
							 (name inspired by Python and Perl) */

	InteractiveSyntaxAnalyzer::Imp *parent;

	void argument(Simple::Element *);
};

/**
 * Prepares the analyzer to start waiting for input.
 */
InteractiveSyntaxAnalyzer::InteractiveSyntaxAnalyzer()
	: imp(0)
{
	enter();
}

/**
 * Sets the namespace containing the names of elements
 * to invoke (functions and classes).
 */
void InteractiveSyntaxAnalyzer::have(Handle<Robin::Namespace> globals)
{
	imp->globals.push_back(globals);
}

/**
 */
void InteractiveSyntaxAnalyzer::import()
{
	imp->activity = Imp::ACT_IMPORT;
}

/**
 */
void InteractiveSyntaxAnalyzer::identifier(const char *name)
{
	if (imp->activity == Imp::ACT_IMPORT) {
		// The identifier is a module name
		imp->function = strdup(name);
	}
	else if (!imp->args_mode) {
		// The identifier is the name of a function
		imp->activity = Imp::ACT_FUNCALL;
		imp->function = strdup(name);
		imp->args_mode = true;
	}
	else {
		// The identifier denotes a constant (enumerated type)
		try {
			if (imp->locals) {
				// This is a name of an enumerated literal
				imp->argument(new Robin::SimpleEnumeratedConstantElement
							  (imp->locals, imp->locals->lookup(name)));
				imp->locals = Handle<Robin::EnumeratedType>();
			}
			else {
				// This is the name of an enum
				imp->locals = multipleLookup<Robin::EnumeratedType>
					(imp->globals.begin(), imp->globals.end(), lookupEnumIn, 
					 name);
			}
		}
		catch (std::exception& e) {
			std::cerr << "// @ERROR: (in enum expression) " << e.what()
					  << std::endl;
		}
	}
}

void InteractiveSyntaxAnalyzer::Imp::argument(Simple::Element *arg)
{
	if (args_mode) {
		args.push_back((Robin::scripting_element)arg);
		personal_args.push_back(arg);
	}
	else {
		personal_instance = arg;
	}
}

/**
 */
void InteractiveSyntaxAnalyzer::var(int cell)
{
	imp->destination = cell;
}

/**
 */
void InteractiveSyntaxAnalyzer::literal(int value)
{
	imp->argument(Simple::build(value));
}

/**
 */
void InteractiveSyntaxAnalyzer::literal(float value)
{
	imp->argument(Simple::build(value));
}

/**
 */
void InteractiveSyntaxAnalyzer::literal(std::string value)
{
	imp->argument(Simple::buildElement<Simple::String>(value));
}

/**
 */
void InteractiveSyntaxAnalyzer::reuse()
{
	if (imp->_)
		imp->argument(imp->_);
	else
		std::cerr << "// @WARNING: no value in '_'." << std::endl;
}

/**
 * Uses the value of a variable as an argument to a function call.
 */
void InteractiveSyntaxAnalyzer::reuse(int cell)
{
	imp->argument(imp->cells[cell]);
}

/**
 * Uses the address of a variable as an argument to a function call.
 */
void InteractiveSyntaxAnalyzer::reuse_address(int cell)
{
	Simple::Element *lvalue = imp->cells[cell];
	Simple::Integer *lvalue_int;
	Simple::String *lvalue_string;
	Robin::Address *address = NULL;

	if (lvalue_int = dynamic_cast<Simple::Integer*>(lvalue))
		address = new Robin::Address(Robin::ArgumentInt, &(lvalue_int->value));
	else if (lvalue_string = dynamic_cast<Simple::String*>(lvalue))
		address = new Robin::Address(Robin::ArgumentCString,
									 new const char *(lvalue_string->value.c_str()));

	if (address != NULL) {
		Handle<Robin::Address> haddress(address);
		imp->argument(new Robin::SimpleAddressElement(haddress));
	}
	else {
		std::cerr << "// @WARNING: can't take address of "
				  << char('A' + cell) << "." << std::endl;
	}
}

/**
 * Uses a datum pointed to by an address as an argument to a function
 * call.
 */
void InteractiveSyntaxAnalyzer::reuse_by_address(int cell)
{
	Robin::SimpleAddressElement *element =
		dynamic_cast<Robin::SimpleAddressElement *>(imp->cells[cell]);

	if (element) {
		imp->argument((Simple::Element*)(element->value->dereference()));
	}
	else {
		std::cerr << "// @WARNING: can't dereference "
				  << char('A' + cell) << ", not an address." << std::endl;
	}
}


/**
 */
void InteractiveSyntaxAnalyzer::enter()
{
	Imp *nimp = new Imp;
	if (imp) *nimp = *imp;
	nimp->parent = imp;
	nimp->activity = Imp::ACT_NOTHING;
	nimp->args.resize(0);
	nimp->personal_args.resize(0);
	nimp->args_mode = false;
	nimp->personal_instance = NULL;
	nimp->function = NULL;
	nimp->destination = -1;
	nimp->_ = NULL;
	imp = nimp;
}

/**
 */
void InteractiveSyntaxAnalyzer::exit()
{
	// Close any unclosed statements
	if (imp->function) period();
	// Discard the current scope
	Simple::Element *inner_result = imp->_;
	Imp *container = imp->parent;
	delete imp;
	if ((imp = container) == NULL) ::exit(0);
	// Give the result of this scope back up to calling scope
	if (inner_result) imp->argument(inner_result);
}

/**
 */
void InteractiveSyntaxAnalyzer::period()
{
	switch (imp->activity) {
	case Imp::ACT_NOTHING:   actNothing();             break;
	case Imp::ACT_FUNCALL:   actFunctionCall();        break;
	case Imp::ACT_IMPORT:    actImportModule();        break;
	}

	// Do some cleanup and prepare for next statement
	imp->activity = Imp::ACT_NOTHING;
	imp->destination = -1;
	if (imp->function) { free(imp->function); imp->function = NULL; }
	imp->args_mode = false;
	imp->args.resize(0);
	imp->personal_args.resize(0);
	imp->personal_instance = NULL;
}

#define REPORT(label, V) \
if (imp->parent) \
dbg::trace << "// @" << label << ": " << (V) << dbg::endl; \
else \
std::cerr << "// @" << label << ": " << (V) << std::endl;

/**
 * Merely returns the value given without manipulation.
 */
void InteractiveSyntaxAnalyzer::actNothing()
{
	if (imp->personal_instance) {
		REPORT("VALUE", *(imp->personal_instance));
		imp->_ = imp->personal_instance;
		if (imp->destination >= 0) {
			imp->cells[imp->destination] = imp->_;
		}
	}
}

/**
 */
void InteractiveSyntaxAnalyzer::actFunctionCall()
{
	if (imp->function) {
		// Debug trace
		dbg::trace << "// @CALLING: '" << imp->function << "'";
		for (PersonalArgumentList::const_iterator
			 argi = imp->personal_args.begin();
			 argi != imp->personal_args.end(); ++argi)		{
			dbg::trace << " [" << **argi << "]";
		}
		dbg::trace << dbg::endl;

		// Attempt to carry out the statement
		try {
			Robin::scripting_element result;
			if (!imp->personal_instance) {
				try {
					// Lookup class
					Handle<Robin::Class> klass = 
						multipleLookup<Robin::Class>
						(imp->globals.begin(), imp->globals.end(),
						 lookupClassIn, imp->function);
					// Create instance!
					result = wrapInstance(klass->createInstance(imp->args));
				}
				catch (Robin::LookupFailureException& e) {
					// Lookup function
					Handle<Robin::Callable> func =
						multipleLookup<Robin::Callable>
						(imp->globals.begin(), imp->globals.end(),
						 lookupRoutineIn, imp->function);
					// Invoke!
					result = func->call(imp->args);
				}
			}
			else {
				// Invoke an instance method
				Robin::SimpleInstanceObjectElement *instelem =
					dynamic_cast<Robin::SimpleInstanceObjectElement*>
					     (imp->personal_instance);
				if (!instelem) throw NotAnInstanceException();
				// Bound method from class
				Handle<Robin::Callable> bmethod =
					instelem->value->bindMethod(imp->function);
				// Invoke!
				result = bmethod->call(imp->args);
			}
			// Print outcome
			if (result != NONE) {
				REPORT("RETURNED", *(Simple::Element*)result);
				// Store in '_'
				imp->_ = (Simple::Element*)result;
				// Store in destination cell, if any
				if (imp->destination >= 0) {
					imp->cells[imp->destination] = imp->_;
				}
			}
		}
		catch (Robin::UserExceptionOccurredException& ue) {
			std::cerr << "// @ERROR: " << ue.what() << std::endl
					  << "// @WHAT: " << ue.user_what << std::endl;
		}
		catch (std::exception& e) {
			std::cerr << "// @ERROR: " << e.what() << std::endl;
		}
	}
}

/**
 * imports the module denoted by "imp->function". The
 * extension ".so" is attached to the filename.
 */
void InteractiveSyntaxAnalyzer::actImportModule()
{
	if (!imp->function) {
		std::cerr << "// @ERROR: no name for import." << std::endl;
		return ;
	}

	try {
		std::string libfile = imp->function;
		// Load library using the registration mechanism
		Handle<Robin::Library> lib = 
			Robin::RegistrationMechanismSingleton::getInstance()
			->import(libfile + ".so");
		have(lib->globalNamespace());
	}
	catch (std::exception& e) {
		std::cerr << "// @ERROR: " << e.what() << std::endl;
	}
}

/**
 */
Simple::Element *InteractiveSyntaxAnalyzer::result() const
{
	return imp->_;
}
