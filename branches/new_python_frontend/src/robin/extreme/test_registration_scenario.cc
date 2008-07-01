/* -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @par SOURCE
 * extreme/test_registration_scenario.cc
 *
 * @par TITLE
 * Scenario Testing
 *
 * @par PACKAGE
 * Robin
 *
 * Relies on the registration mechanism to provide the enterprise
 * library, and attempts a simple scenario which does some actions on the
 * hotel database. The answers are then compared with the expected figures
 * to make sure they are correct.
 */

#include "fwtesting.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sstream>

#include <pattern/attached_fstream.h>

#include <robin/reflection/library.h>
#include <robin/frontends/simple/elements.h>

#include "reflectiontest.h"
#include "interactive/syntax.h"


extern Handle<Robin::Library> enterprise;


class TestRegistrationScenario1 : public Extreme::Test
{
private:
	struct byHotel {
		int Fujiyama;
		int Royal;
		int Boomerang;
	} avail_rooms, avail_beds;

	int remaining_unoccupied;

	friend bool operator==(const byHotel& a, const byHotel& b)
	{ return (a.Fujiyama == b.Fujiyama && a.Royal == b.Royal && 
			  a.Boomerang == b.Boomerang); }

	static const char *const CORP_FL;
	static const char *const ROOMS_FL;

	InteractiveSyntaxAnalyzer interpreter;

protected:
	SpontaneousElementExtractor<Simple::Integer, int> Gint;

public:
	TestRegistrationScenario1() : Extreme::Test("Reg. Scenario #1") { }

	void prepare() {
		interpreter.have(enterprise->globalNamespace());

		FILE *corp, *rooms;
		if ((corp = fopen(CORP_FL, "r")) == 0) { perror("corp"); abort(); }
		if ((rooms = fopen(ROOMS_FL, "r")) == 0) { perror("rooms"); abort(); }
		// Run the database scripts to create an initial set of hotels
		interpret("flush.");  /* Make sure the database is empty */
		interpret(corp);
		interpret(rooms);
		fclose(corp); fclose(rooms);
	}

	void go() {
		interpret("(");
		struct Grd {
			TestRegistrationScenario1 *scenario; 
			~Grd() { scenario->interpret(")"); } 
		} body_guard = { this };
		// Rent one room in each hotel
		interpret("(findHotel \"Fujiyama\")  checkin RoomType TOURIST 2.");
		interpret("(findHotel \"Royal\")     checkin RoomType EMBASSY 1.");
		interpret("(findHotel \"Boomerang\") checkin RoomType TOURIST 3.");
		// Get information back from the hotels
		interpret("(findHotel \"Fujiyama\") availableRooms.");
		avail_rooms.Fujiyama = Gint | result();
		interpret("(findHotel \"Fujiyama\") availableBeds.");
		avail_beds.Fujiyama = Gint | result();

		interpret("(findHotel \"Royal\") availableRooms.");
		avail_rooms.Royal = Gint | result();
		interpret("(findHotel \"Royal\") availableBeds.");
		avail_beds.Royal = Gint | result();

		interpret("B: 0. C: 0. (findHotel \"Boomerang\") available &B &C.");
		interpret("B.");
		avail_rooms.Boomerang = Gint | result();
		interpret("C.");
		avail_beds.Boomerang = Gint | result();
	}

	bool verify() {
		// We know which results to expect, so just -
		byHotel expected_rooms = { 3, 3, 2 },
			expected_beds = { 5, 14, 6 };          // ^ ^  some may accuse me
		return (avail_rooms == expected_rooms &&   //  _   of hard-coding...
				avail_beds == expected_beds);      //      I hope you won't.
	}


	void interpret(FILE *input) {
		std::istream *stream = new Pattern::attached_ifstream(input, 1);
		interpreter.yyrestart(stream);
		interpreter.yylex();
		delete stream;
	}

	void interpret(int filedes) {
		FILE *file = fdopen(filedes, "r");
		interpret(file);
	}

	void interpret(std::string command) {
		std::istringstream stream(command);
		interpreter.yyrestart(&stream);
		interpreter.yylex();
	}

	Simple::Element *result()  {
		return interpreter.result();
	}

} __rgs01;


const char *const 
TestRegistrationScenario1::CORP_FL = "src/robin/extreme/corp";
const char *const 
TestRegistrationScenario1::ROOMS_FL = "src/robin/extreme/rooms";

