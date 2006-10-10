// -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-

/**
 * @par SOURCE
 * test_reflection_lowmed.cc
 *
 * @par TITLE
 * Low and Medium
 *
 * Tests the three calling conventions:
 * <ul><li>Low level</li><li>Medium level</li><li>High level</li></ul>
 */

#include <robin/reflection/typeofargument.h>
#include <robin/reflection/cfunction.h>
#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/overloadedset.h>
#include <robin/reflection/instance.h>
#include <robin/reflection/address.h>
#include <robin/frontends/framework.h>
#include <robin/frontends/simple/elements.h>
#include <robin/frontends/simple/instanceelement.h>

#include <robin/debug/trace.h>

#include "fwtesting.h"
#include "reflectiontest.h"
#include "test_reflection_arena.h"

using namespace Extreme;

/**
 * A sample 'external' function for the testing.
 */
int callme(long x)
{
    return x * x - x / 7;
}

int callme_ul(unsigned long x)
{
    return x * x - x / 7;
}

int callme_c(char x)
{
    return x * x - x / 7;
}

long callme_f(float x)
{
	return long(x * x - x / 7);
}


int callmeo1() { return 1; }
int callmeo2(int) { return 2; }
int callmeo3(long) { return 3; }
int callmeo4(int, int) { return 4; }

Robin::scripting_element ReflectionTest::randomElementInt()
{
    return (Robin::scripting_element)Simple::build((int)rand());
}

Robin::scripting_element ReflectionTest::randomElementLong()
{
    return (Robin::scripting_element)Simple::build((long)rand());
}

Simple::Element *
ReflectionTest::simpleInstanceElement(Handle<Robin::Instance> instance)
{
	return 
		new Robin::SimpleInstanceObjectElement(instance);
}

Simple::Element *
ReflectionTest::simpleInstanceElement(Handle<Robin::Class> klass, 
									  void *cppobject)
{
	Handle<Robin::Instance> instance(new Robin::Instance(klass, cppobject));
	return simpleInstanceElement(instance);
}

Robin::scripting_element 
ReflectionTest::instanceElement(Handle<Robin::Instance> instance)
{
	return (Robin::scripting_element)
		(new Robin::SimpleInstanceObjectElement(instance));
}

Robin::scripting_element 
ReflectionTest::instanceElement(Handle<Robin::Class> klass, 
								void *cppobject)
{
	Handle<Robin::Instance> instance(new Robin::Instance(klass, cppobject));
	return instanceElement(instance);
}

template < class E >
E manual_reinterpret(Robin::basic_block rvalue)
{
	return reinterpret_cast<E&>(rvalue);
}

/**
 * @par TEST
 * TestLowLevel
 *
 * Low-level calling convention.
 */
class TestLowLevel : public Test
{
    Robin::CFunction func1;
    Robin::CFunction func2;
    Robin::CFunction func3;
    long argument;
    int  return_value1;
    int  return_value2;
    int  return_value3;
    int  alt_return_value;

public:
    TestLowLevel() : Test("low-level"), func1((Robin::symbol)&callme),
					 func2((Robin::symbol)&callme_ul),
					 func3((Robin::symbol)&callme_c) { }

    void prepare() {
		argument = 70;
    }

    void go() {
		Robin::ArgumentsBuffer args1, args2, args3;
		args1.pushInt(argument);
		args2.pushLong(argument);
		args3.pushChar(argument);
		return_value1 = manual_reinterpret<int>(func1.call(args1));
		return_value2 = manual_reinterpret<int>(func2.call(args2));
		return_value3 = manual_reinterpret<int>(func3.call(args3));
    }

    void alternate() {
		alt_return_value = callme(argument);
    }
  
    bool verify() {
		notification("RETURN-VALUE1", return_value1);
		notification("RETURN-VALUE2", return_value2);
		notification("RETURN-VALUE3", return_value3);
		notification("ALT-RETURN-VALUE", alt_return_value);
		return (return_value1 == alt_return_value) &&
			(return_value2 == alt_return_value) && 
			(return_value3 == alt_return_value);
    }
} __t01;

/**
 * @par TEST
 * TestMediumLevel
 *
 * Medium-level calling convention.
 */
class TestMediumLevel : public ReflectionTest
{
    Robin::CFunction func1;
    Robin::CFunction func2;
    Robin::CFunction func3;
    long argument;
    int return_value1;
    int return_value2;
    int return_value3;
    int alt_return_value;

public:
    TestMediumLevel() : ReflectionTest("medium-level"),
			func1((Robin::symbol)&callme),
			func2((Robin::symbol)&callme_ul),
			func3((Robin::symbol)&callme_c) { }

    void prepare() {
		func1.specifyReturnType(Robin::ArgumentInt);
		func1.addFormalArgument(Robin::ArgumentLong);
		func2.specifyReturnType(Robin::ArgumentInt);
		func2.addFormalArgument(Robin::ArgumentULong);
		func3.specifyReturnType(Robin::ArgumentInt);
		func3.addFormalArgument(Robin::ArgumentChar);
		argument = rand() % 120;
    }

    int element2int(Robin::scripting_element e) 
	{
		Simple::Element *return_element = (Simple::Element *)e;
		Simple::Integer *return_integer =
			dynamic_cast<Simple::Integer*>(return_element);
		return return_integer->value;		
    }

    void go() {
		Robin::ActualArgumentList args1, args2, args3;
		args1.push_back((Robin::scripting_element)Simple::build(argument));
		args2.push_back((Robin::scripting_element)
						Simple::build((long)argument));
		args3.push_back((Robin::scripting_element)
						Simple::build((char)argument));
		return_value1 = Gint | (func1.call(args1));
		return_value2 = Gint | (func2.call(args2));
		return_value3 = Gint | (func3.call(args3));
    }

    void alternate() {
		alt_return_value = callme(argument);
    }

    bool verify() {
		return (return_value1 == alt_return_value) &&
			(return_value2 == alt_return_value) && 
			(return_value3 == alt_return_value);
    }

} __t02;


/**
 * @par TEST
 * TestOverloading
 *
 * Overloaded function call. There is a sample function
 * with several alternatives:
 * <ul>
 *  <li>int f()</li>
 *  <li>int f(int)</li>
 *  <li>int f(long)</li>
 *  <li>int f(int, int)</li>
 * </ul>
 * The test randomly chooses an actual argument list which matches
 * one of the above exactly, and calls the function. The implementation
 * of the function is designed in a way that it returns the number of
 * the alternative selected, which can be compared with the randomly
 * chosen one.
 */
class TestOverloading : public ReflectionTest
{
    Handle<Robin::CFunction> alt1;
    Handle<Robin::CFunction> alt2;
    Handle<Robin::CFunction> alt3;
    Handle<Robin::CFunction> alt4;
    Robin::OverloadedSet func;

    int chosen;
    Robin::ActualArgumentList args;

    int returned;

public:
    TestOverloading() : ReflectionTest("simple overloading"),
	  alt1(new Robin::CFunction((Robin::symbol)&callmeo1)),
	  alt2(new Robin::CFunction((Robin::symbol)&callmeo2)),
	  alt3(new Robin::CFunction((Robin::symbol)&callmeo3)),
	  alt4(new Robin::CFunction((Robin::symbol)&callmeo4)) {
		// Fill overloaded alternatives
		alt2->addFormalArgument(Robin::ArgumentInt);
		alt3->addFormalArgument(Robin::ArgumentLong);
		alt4->addFormalArgument(Robin::ArgumentInt);
		alt4->addFormalArgument(Robin::ArgumentInt);
		// Set the return type of all to int
		alt1->specifyReturnType(Robin::ArgumentInt);
		alt2->specifyReturnType(Robin::ArgumentInt);
		alt3->specifyReturnType(Robin::ArgumentInt);
		alt4->specifyReturnType(Robin::ArgumentInt);
		// Add all the alternatives
		func.addAlternative(alt1); func.addAlternative(alt2);
		func.addAlternative(alt3); func.addAlternative(alt4);
    }

    void prepare() {
		chosen = rand() % 4 + 1;
		dbg::trace << "// @CHOSEN: " << chosen << dbg::endl;
		args = Robin::ActualArgumentList();
		switch (chosen) {
		case 4: args.push_back(randomElementInt());  // and fall through
		case 2: args.push_back(randomElementInt());
			break;
		case 3: args.push_back(randomElementLong());
			break;
		}
    }

    void go() {
		returned = Gint | func.call(args);
		dbg::trace << "// @SELECTED: " << returned << dbg::endl;
    }

    bool verify() {
		return (returned == chosen);
    }

} __t03;


/**
 * @par TEST
 * TestImplicitConversion
 *
 * Tests the conversion of intrinsic and user-defined
 * types in function calls.
 */
class TestImplicitConversion : public ReflectionTest
{
private:
	Handle<Robin::CFunction> func;
	Handle<Robin::OverloadedSet> ofunc;
	int argument;
	Room *room04;

	long returned;
	long alt_returned;

	long returned_oop;
	long alt_returned_oop;

public:
	TestImplicitConversion() : ReflectionTest("Implicit Conversions"),
				func(new Robin::CFunction((Robin::symbol)&callme_f)),
			    ofunc(new Robin::OverloadedSet),
				room04(new Room) { }

	void prepare() {
		func->specifyReturnType(Robin::ArgumentLong);
		func->addFormalArgument(Robin::ArgumentFloat);
		ofunc->addAlternative(func);
		argument = 56;
		room04->m_level = EMBASSY;
		room04->m_capacity = 9;
		room04->m_poolside = true;
		notification("ARGUMENT", argument);
		notification("ROOM", *room04);
	}

	void go() {
		// Call the callme_f function - assume a conversion from long to float
		Robin::ActualArgumentList args;
		args.push_back((Robin::scripting_element)Simple::build(argument));
		returned = Glong | ofunc->call(args);
		notification("RETURNED", returned);
		// Call the callme_f function - assume a conversion from Room to int,
		// and from int to float
		Robin::ActualArgumentList args_oop;
		args_oop.push_back(instanceElement(enterprise_Room, room04));
		returned_oop = Glong | ofunc->call(args_oop);
		notification("RETURNED-OOP", returned_oop);
	}

	void alternate() {
		alt_returned = callme_f(argument);
		notification("ALT-RETURNED", alt_returned);
		alt_returned_oop = callme_f(room04->m_capacity);
		notification("ALT-RETURNED-OOP", alt_returned_oop);
	}

	bool verify() {
		return (returned == alt_returned) && 
			(returned_oop == alt_returned_oop);
	}

} __t04;


/**
 * @par TEST
 * TestAddressBasic
 *
 * Tests the Address class.
 */
class TestAddressBasic : public ReflectionTest
{
private:
	Handle<Robin::Address> int_address;
	int int_var;
	int int_val;
	Handle<Robin::Address> float_address;
	float float_var;
	float float_val;

public:
	TestAddressBasic() : ReflectionTest("Basic Addressing")
	{
	}

	void prepare() { 
		Robin::Address *address;

		address = new Robin::Address(Robin::ArgumentInt, &int_var);
		int_address = Handle<Robin::Address>(address);
		address = new Robin::Address(Robin::ArgumentFloat, &float_var);
		float_address = Handle<Robin::Address>(address);

		int_var = 71908;
		notification("INT-VAR", int_var);
		float_var = 719.08;
		notification("FLOAT-VAR", float_var);
	}

	void go() { 
		int_val = Gint | int_address->dereference();
		float_val = Gfloat | float_address->dereference();
		notification("INT-VAL", int_val);
		notification("FLOAT-VAL", float_val);
	}

	void alternate() { }

	bool verify() { return int_var == int_val && float_var == float_val; }

} __t05;



long assignme(long *l, long r)
{
	*l = r;
	return reinterpret_cast<long>(l);
}


class TestPointerArgument : public ReflectionTest
{
private:
	Robin::CFunction func1;
	long long_var;
	long long_val;
	long returned1;

public:
	TestPointerArgument() : ReflectionTest("Pointer Argument"),
			func1((Robin::symbol)&assignme)
	{
	}

	void prepare() { 
		Robin::FrontendsFramework::fillAdapter(Robin::ArgumentLong->pointer());

		func1.specifyReturnType(Robin::ArgumentLong);
		func1.addFormalArgument(Robin::ArgumentLong->pointer());
		func1.addFormalArgument(Robin::ArgumentLong);

		long_val = 90210;
		long_var = 0;
		notification("LONG-VAL", long_val);
	}

	void go() { 
		Robin::Address *address;
		address = new Robin::Address(Robin::ArgumentLong, &long_var);
		Handle<Robin::Address> haddress(address);
		
		Robin::ActualArgumentList args1;
		args1.push_back(new Robin::SimpleAddressElement(haddress));
		args1.push_back(Simple::build(long_val));
		returned1 = Glong | func1.call(args1);
		notification("LONG-VAR", long_var);
		notification("RETURNED", returned1);
	}

	void alternate() { }

	bool verify() { return long_var == long_val &&
						returned1 == reinterpret_cast<long>(&long_var); }

} __t06;
