// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @par SOURCE
 * extreme/test_reflection_arena.cc
 *
 * @par TITLE
 * Reflection Test - sample library
 *
 * @par PACKAGE
 * Robin
 */

#include <iostream>
#include <iomanip>
#include <robin/debug/assert.h>

#include "test_reflection_arena.h"

#include <robin/reflection/intrinsic_type_arguments.h>
#include <robin/reflection/cfunction.h>
#include <robin/reflection/conversiontable.h>
#include <robin/reflection/fundamental_conversions.h>
#include <robin/reflection/namespace.h>

#include <robin/frontends/framework.h>
#include <robin/frontends/simple/elements.h>
#include <robin/frontends/simple/instanceelement.h>

using Robin::CFunction;
using Robin::symbol;


Handle<Robin::EnumeratedType> enterprise_RoomType
    (new Robin::EnumeratedType("RoomType"));
Handle<Robin::Class> enterprise_string;
Handle<Robin::Class> enterprise_Location;
Handle<Robin::Class> enterprise_Room;
Handle<Robin::Class> enterprise_Hotel;

// function prototypes
void introduceConversion(Handle<Robin::RobinType> from,
						 Handle<Robin::RobinType> to,
						 Robin::Conversion *conv_type);


class SampleConversionRoom2Int : public Robin::Conversion
{
public:
	SampleConversionRoom2Int() 
	{ setWeight(Robin::Conversion::Weight(1,0,0,0)); }

	Robin::scripting_element apply(Robin::scripting_element value) const
	{
		Simple::Element *element = (Simple::Element *)value;
		Robin::SimpleInstanceObjectElement *ioelement =
			dynamic_cast<Robin::SimpleInstanceObjectElement *>(element);
		assert(ioelement);
		// Now - this must be a Room! Get it.
		assert(ioelement->value->getClass() == enterprise_Room);
		Room *room = (Room *)(ioelement->value->getObject());
		// Build an integer
		Simple::Element *converted = Simple::build((int)room->m_capacity);
		return (Robin::scripting_element)converted;
	}
};


class SampleConversionCString2stdstring : public Robin::Conversion
{
public:
	SampleConversionCString2stdstring()
	{ setWeight(Robin::Conversion::Weight(1,0,0,0)); }

	Robin::scripting_element apply(Robin::scripting_element value) const
	{
		Simple::Element *element = (Simple::Element *)value;
		Simple::String *str = dynamic_cast<Simple::String*>(element);
		assert(str);
		// Now - create an std::string
		std::string *strinstance = new std::string(str->value);
		// Build an instance
		Handle<Robin::Instance> instance(
			new Robin::Instance(enterprise_string, (void*)strinstance));
		Simple::Element *converted = 
			new Robin::SimpleInstanceObjectElement(instance);
		return (Robin::scripting_element)converted;
	}
};

/**
 */
Handle<Robin::Class> declareClass(const char *name)
{
	Handle<Robin::Class> decl = Robin::Class::create_new(name);
	// Enable converting from a reference to a pointer (trivially).
	// This is needed so any functions accepting pointers can ever
	// be called.
	introduceConversion(decl->getType(), decl->getPtrType(),
						new Robin::TrivialConversion);
	
	return decl;
}

/**
 */
void introduceConversion(Handle<Robin::RobinType> from,
						 Handle<Robin::RobinType> to,
						 Robin::Conversion *conv_type)
{
	Handle<Robin::Conversion> hconv(conv_type);
	hconv->setSourceType(from);
	hconv->setTargetType(to);
	Robin::ConversionTableSingleton::getInstance()
		->registerConversion(hconv);
}

/**
 */
void fillAdapter(Handle<Robin::RobinType> toa)
{
	assert(toa);
	toa->assignAdapter(
	    Robin::FrontendsFramework::activeFrontend()->giveAdapterFor(*toa));
}

/**
 * Makes all the hotel enterprise's classes and types,
 * including constructors and instance methods.
 */
Handle<Robin::Namespace> registerEnterprise()
{
	Handle<Robin::Namespace> ns(new Robin::Namespace("<default>"));

	enterprise_string =   declareClass("std::string");
	enterprise_Location = declareClass("Location");
	enterprise_Room =     declareClass("Room");
	enterprise_Hotel =    declareClass("Hotel");
	enterprise_RoomType->activate(enterprise_RoomType);

	// Register string
	Handle<CFunction> hstr(new CFunction((symbol)&string_new,"string_new",CFunction::Constructor,"std::string"));

	hstr->addFormalArgument(Robin::ArgumentCString);

	enterprise_string->addConstructor(hstr);

	// Register Hotel
	Handle<CFunction> hnew(new CFunction((symbol)&Hotel_new,"Hotel_new",CFunction::Constructor,enterprise_Hotel->name()));
	Handle<CFunction> hcpy(new CFunction((symbol)&Hotel_copy,"Hotel_copy",CFunction::Constructor,enterprise_Hotel->name()));
	Handle<CFunction> hname(new CFunction((symbol)&Hotel_getName,"Hotel_getName",CFunction::Method,enterprise_Hotel->name()));
	Handle<CFunction> hloc(new CFunction((symbol)&Hotel_getLocation,"Hotel_getLocation",CFunction::Method,enterprise_Hotel->name()));
	Handle<CFunction> hroom(new CFunction((symbol)&Hotel_getRoomNo,"Hotel_getRoomNo",CFunction::Method,enterprise_Hotel->name()));

	hnew->addFormalArgument(enterprise_string->getConstType());
	hnew->addFormalArgument(enterprise_Location->getConstType());

	hcpy->addFormalArgument(enterprise_Hotel->getConstType());

	hname->addFormalArgument(enterprise_Hotel->getPtrType());
	hname->specifyReturnType(Robin::ArgumentCString);

	hloc->addFormalArgument(enterprise_Hotel->getPtrType());
	hloc->specifyReturnType(enterprise_Location->getConstType());

	hroom->addFormalArgument(enterprise_Hotel->getPtrType());
	hroom->addFormalArgument(Robin::ArgumentInt);
	hroom->specifyReturnType(enterprise_Room->getConstType());

	enterprise_Hotel->addConstructor(hnew);
	enterprise_Hotel->addConstructor(hcpy);
	enterprise_Hotel->addInstanceMethod("getName", hname);
	enterprise_Hotel->addInstanceMethod("getLocation", hloc);
	enterprise_Hotel->addInstanceMethod("getRoomNo", hroom);

	// Register Room
	Handle<CFunction> hroomnew(new CFunction((symbol)&Room_new,"Room_new",CFunction::Constructor,enterprise_Hotel->name()));
	Handle<CFunction> hlevel(new CFunction((symbol)&Room_getLevel,"Romm_getLevel",CFunction::Method,enterprise_Hotel->name()));

	hroomnew->addFormalArgument(enterprise_RoomType->getType());

	hlevel->addFormalArgument(enterprise_Room->getPtrType());
	hlevel->specifyReturnType(enterprise_RoomType->getType());

	enterprise_Room->addConstructor(hroomnew);
	enterprise_Room->addInstanceMethod("getLevel", hlevel);

	introduceConversion(enterprise_Room->getConstType(),
						Robin::ArgumentInt,
						new SampleConversionRoom2Int);
	introduceConversion(Robin::ArgumentCString,
						enterprise_string->getConstType(),
						new SampleConversionCString2stdstring);

	// Register Location
	{
		Handle<CFunction> hloc(new CFunction((symbol)&Location_new,"Location_new",CFunction::Constructor,enterprise_Hotel->name()));
		Handle<CFunction> hprint(new CFunction((symbol)&Location_print,"Location_print",CFunction::Method,enterprise_Hotel->name()));

		hloc->addFormalArgument(enterprise_string->getConstType());
		hloc->addFormalArgument(enterprise_string->getConstType());

		hprint->addFormalArgument(enterprise_Location->getPtrType());

		enterprise_Location->addConstructor(hloc);
		enterprise_Location->addInstanceMethod("print", hprint);
	}

	// Register RoomType
	enterprise_RoomType->addConstant("TOURIST", (int)TOURIST);
	enterprise_RoomType->addConstant("LEISURE", (int)LEISURE);
	enterprise_RoomType->addConstant("SUITE",   (int)SUITE);
	enterprise_RoomType->addConstant("EMBASSY", (int)EMBASSY);

	// Register global functions
	Handle<CFunction> hbuy(new CFunction((symbol)&global_buy,"global_buy",CFunction::GlobalFunction));
	Handle<CFunction> hlist(new CFunction((symbol)&global_list,"global_list",CFunction::GlobalFunction));

	hbuy->addFormalArgument(enterprise_Hotel->getConstType());

	// Apply adapters
	// (ordinarily, this is done by the Registration Mechanism)
	fillAdapter(enterprise_Hotel->getPtrType());
	fillAdapter(enterprise_Hotel->getConstType());
	fillAdapter(enterprise_Location->getPtrType());
	fillAdapter(enterprise_Location->getConstType());
	fillAdapter(enterprise_Room->getPtrType());
	fillAdapter(enterprise_Room->getConstType());
	fillAdapter(enterprise_string->getPtrType());
	fillAdapter(enterprise_string->getConstType());
	fillAdapter(enterprise_RoomType->getType());

	// No - put everything in the namespace and off we go
	ns->declare("Hotel", enterprise_Hotel);
	ns->declare("Location", enterprise_Location);
	ns->declare("Room", enterprise_Room);
	ns->declare("string", enterprise_string);
	ns->declare("RoomType", enterprise_RoomType);

	Handle<Robin::OverloadedSet> hbug_over = Robin::OverloadedSet::create_new("buy");
	ns->declare("buy", static_hcast<Robin::Callable>(hbug_over));

	Handle<Robin::OverloadedSet> hlist_over = Robin::OverloadedSet::create_new("list");
	ns->declare("list", static_hcast<Robin::Callable>(hlist_over));

	return ns;
}
