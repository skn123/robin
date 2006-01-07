// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

#ifndef EXTREME_REFLECTIONTEST_CLASS
#define EXTREME_REFLECTIONTEST_CLASS

#include "fwtesting.h"
#include <exception>
#include <robin/reflection/low_level.h>
#include <robin/frontends/simple/elements.h>
#include <robin/frontends/simple/instanceelement.h>

/**
 * @class SpontaneousElementExtractor
 * @nosubgrouping
 *
 * A generic implementation which knows how to get it
 * from Simple::Element objects.
 */
template <class ElementType, class NativeType>
class SpontaneousElementExtractor
{
public:
	class UnexpectedReturnType : public std::exception
	{
	public:
		const char *what() const throw() { return "unexpected return type."; }
	};

	NativeType extract(Simple::Element *e) const
	{
		ElementType *ce = dynamic_cast<ElementType*>(e);
		if (!ce) throw UnexpectedReturnType();
		NativeType value = ce->value;
		delete ce;
		return value;
	}

	NativeType extract(Robin::scripting_element e) const
	{
		return extract((Simple::Element*)e);
	}

	// syntactic sugaring
	NativeType operator | (Robin::scripting_element e) const 
	{ return extract(e); }

	NativeType operator | (Simple::Element *e) const
	{ return extract(e); }
};


/**
 * @class ReflectionTest
 * @nosubgrouping
 *
 * Base class for all reflection tests, provides basic
 * utilities for testing the reflection.
 */
class ReflectionTest : public Extreme::Test
{
public:
	ReflectionTest() { }
	ReflectionTest(const char *title) : Extreme::Test(title) { }

    static Robin::scripting_element randomElementInt();
	static Robin::scripting_element randomElementLong();

	static Robin::scripting_element
		instanceElement(Handle<Robin::Instance>);
	static Robin::scripting_element
		instanceElement(Handle<Robin::Class>, void *cppobject);
	static Simple::Element *
		simpleInstanceElement(Handle<Robin::Instance> instance);
	static Simple::Element *
		simpleInstanceElement(Handle<Robin::Class> klass, 
							  void *cppobject);
	

protected:
	SpontaneousElementExtractor<Simple::Integer, int> Gint;
	SpontaneousElementExtractor<Simple::Long, long> Glong;
	SpontaneousElementExtractor<Simple::String, std::string> Gstring;
	SpontaneousElementExtractor<Robin::SimpleInstanceObjectElement, 
                                Handle<Robin::Instance> >
		Ginstance;
	SpontaneousElementExtractor<Robin::SimpleEnumeratedConstantElement, int>
		Genum;
};



#endif
