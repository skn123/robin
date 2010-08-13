// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @par SOURCE
 * extreme/test_reflection_oop.cc
 *
 * @par TITLE
 * Object-Oriented Features Tests
 *
 * @par PACKAGE
 * Robin
 */

#include <string>
#include <robin/reflection/class.h>
#include <robin/reflection/instance.h>
#include <robin/frontends/simple/elements.h>
#include <robin/frontends/simple/instanceelement.h>

#include "test_reflection_arena.h"

#include "fwtesting.h"
#include "reflectiontest.h"

using namespace Extreme;


/**
 * @par TEST
 * ConstructionTest
 *
 * Tests the ability of a class object to generate
 * instances.
 */
class ConstructionTest : public ReflectionTest
{
protected:
	std::string hotel_name;
	Location hotel_location;
	Handle<Robin::Instance> ihotel;

public:
	ConstructionTest() : ReflectionTest("Class Constructors") { }
	ConstructionTest(const char *title) : ReflectionTest(title) { }

	void prepare() {
		hotel_name = "L'Etoile";
		hotel_location.country = "France";
		hotel_location.city = "Nice";
	}

	void go() {
		Simple::Element *ename = 
			simpleInstanceElement(enterprise_string, &hotel_name);
		Simple::Element *elocation = 
			simpleInstanceElement(enterprise_Location, &hotel_location);
		Handle<Robin::ActualArgumentList> args(new Robin::ActualArgumentList);
		Robin::KeywordArgumentMap empty;
		args->push_back((Robin::scripting_element)ename);
		args->push_back((Robin::scripting_element)elocation);
		ihotel = enterprise_Hotel->createInstance(args,empty);
		// Some cleanup
		delete ename;
		delete elocation;
	}

	bool verify() {
		Hotel *hot = (Hotel *)ihotel->getObject();
		return hotel_name == hot->m_name && 
			hotel_location == hot->m_location;
	}

} __t21;

/**
 * @par TEST
 * TestCopyConstructor
 *
 * Tests Class' ability to clone an instance of itself.
 */
class TestCopyConstructor : public ConstructionTest
{
protected:
	Handle<Robin::Instance> icloned;

public:
	TestCopyConstructor() : ConstructionTest("Class Copy Constructor") { }

	void go() {
		// Build the instance using the regular constructor
		ConstructionTest::go();
		// Clone the instance created by the preceding call
		icloned = enterprise_Hotel->createInstance(*ihotel);
		// Destory the old instance and replace it with the cloned one
		ihotel = icloned;
	}
} __t22;

/**
 * @par TEST
 * TestMethodInvocation
 *
 * Tests the method invocation convention.
 */
class TestMethodInvocation : public ReflectionTest
{
private:
	std::string hotel_name;
	Location hotel_location;
	Handle<Hotel> hotel;
	Room room01;
	Handle<Robin::SimpleInstanceObjectElement> ihotel;

	std::string ename;
	Room eroom;

public:
	TestMethodInvocation() : ReflectionTest("Instance Methods") { }

	void prepare() {
		hotel = Handle<Hotel>(new Hotel);
		hotel->m_name = hotel_name = "Seven Seas";
		hotel_location.country = "England";
		hotel_location.city = "Greenwich";
		hotel->m_location = hotel_location;
		// Add a room to the sample hotel
		room01.m_level = TOURIST;
		room01.m_capacity = 5;
		room01.m_poolside = false;
		hotel->m_rooms.push_back(room01);
		// Create an instance out of the sample

		Handle<Robin::Instance> ihotel_instance = Handle<Robin::Instance>(
								new Robin::Instance(enterprise_Hotel, &*hotel));

		ihotel = Handle<Robin::SimpleInstanceObjectElement>(
				new Robin::SimpleInstanceObjectElement(ihotel_instance));
		notification("HOTEL-NAME", hotel_name);
		notification("ROOM01", room01);
	}

	void go() {
		Robin::KeywordArgumentMap empty;
		// Get the name using getName()
		Handle<Robin::Class> klass  = ihotel->value->getClass();
		Handle<Robin::CallableWithInstance> getName = klass->findInstanceMethod("getName");
		Handle<Robin::ActualArgumentList> args(new Robin::ActualArgumentList);
		ename = Gstring | getName->callUpon(&*ihotel,*args,empty);
		// Get room no. 1 using getRoomNo(i)
		Handle<Robin::CallableWithInstance> getRoomNo = klass->findInstanceMethod("getRoomNo");

		args->push_back((Robin::scripting_element)Simple::build(0));
		eroom = *((Room*)(Ginstance | getRoomNo->callUpon(&*ihotel,*args,empty))->getObject());
		notification("ENAME", ename);
		notification("EROOM", eroom);
	}

	bool verify() {
		return (ename == hotel_name) && (eroom == room01);
	}
} __t23;


/**
 * @par TEST
 * TestEnumeratedInteraction
 *
 * Tests passing enumerated values as arguments and
 * receiving them as return values.
 */
class TestEnumeratedInteraction : public ReflectionTest
{
private:
	RoomType roomlevel;
	Handle<Robin::SimpleInstanceObjectElement> iroom;
	Room* room;

	RoomType eroomlevel;

public:
	TestEnumeratedInteraction() : ReflectionTest("Enumerated Interaction") { }

	void prepare() {
		roomlevel = EMBASSY;
		notification("ROOMLEVEL", roomlevel);
	}

	void go() {
		Handle<Robin::ActualArgumentList> ctor_args(new Robin::ActualArgumentList);
		Robin::KeywordArgumentMap empty;
		ctor_args->push_back((Robin::scripting_element)
		  (new Robin::SimpleEnumeratedConstantElement(enterprise_RoomType,
														roomlevel)));
		Handle<Robin::Instance> iroom_instance = enterprise_Room->createInstance(ctor_args,empty);
		iroom = Handle<Robin::SimpleInstanceObjectElement> (
				new Robin::SimpleInstanceObjectElement(iroom_instance));

		// Get the level of the room by indirectly calling Room::getLevel
		Handle<Robin::CallableWithInstance> getLevel = iroom_instance->getClass()->findInstanceMethod("getLevel");
		Handle<Robin::ActualArgumentList> args(new Robin::ActualArgumentList);
		eroomlevel = (RoomType)(Genum | getLevel->callUpon(&*iroom,*args,empty));
		notification("EROOMLEVEL", eroomlevel);
	}

    bool verify() {
		return (eroomlevel == roomlevel);
	}
} __t24;
