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

#include "enterprise.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <exception>


/**
 * \@GENERAL-PURPOSE OPERATORS
 */

bool operator == (const Location& a, const Location& b)
{
	return (a.country == b.country) && (a.city == b.city);
}

bool operator == (const Room& a, const Room& b)
{
	return (a.m_level == b.m_level) && (a.m_capacity == b.m_capacity) &&
		(a.m_poolside == b.m_poolside);
}

std::ostream& operator << (std::ostream& os, const Location& loc)
{
	return os << loc.city << ", " << loc.country;
}

std::ostream& operator << (std::ostream& os, const Room& room)
{
	os << "level " << room.m_level << " room for " << room.m_capacity;
	if (room.m_poolside) os << " (pool side)";
	return os;
}

/**
 * \@FLAT WRAPPERS
 */

std::string *string_new(const char *cstr)
{
	return new std::string(cstr);
}

Hotel *Hotel_new(std::string& name, Location& location)
{
	Hotel *hotel = new Hotel;
	hotel->m_name = name;
	hotel->m_location = location;
	hotel->m_hasPool = false;
	return hotel;
}

Hotel *Hotel_copy(Hotel& hotel)
{
	return new Hotel(hotel);
}

const char *Hotel_getName(Hotel *self)
{
	return self->m_name.c_str();
}

Location& Hotel_getLocation(Hotel *self)
{
	return self->m_location;
}

Room& Hotel_getRoomNo(Hotel *self, int room_number)
{
	return self->m_rooms[room_number];
}

void Hotel_buildRoom(Hotel *self, Room& room)
{
	self->m_rooms.push_back(room);
}

void Hotel_buildPool(Hotel *self)
{
	self->m_hasPool = true;
}

class NoVacantRoomException : public std::exception
{
public: 
	const char *what() const throw () { return "no vacant room available."; }
};

void Hotel_checkin(Hotel *self, RoomType level, int guests)
{
	Room *best = 0;

	for (std::vector<Room>::iterator ri = self->m_rooms.begin();
		 ri != self->m_rooms.end(); ++ri) {
		// Check room type and occupancy
		if (ri->m_level == level && ri->m_occupied == 0 && 
			ri->m_capacity >= guests &&
			(best == 0 || ri->m_capacity < best->m_capacity)) best = &*ri;
	}

	if (best) {
		best->m_occupied = guests;
	}
	else
		throw NoVacantRoomException();
}

int Hotel_availRooms(Hotel *self)
{
	int vacant_room_count = 0;
	for (std::vector<Room>::iterator ri = self->m_rooms.begin();
		 ri != self->m_rooms.end(); ++ri) {
		// Check occupancy
		if (ri->m_occupied == 0) ++vacant_room_count;
	}

	return vacant_room_count;
}

int Hotel_availBeds(Hotel *self)
{
	int vacant_bed_count = 0;
	for (std::vector<Room>::iterator ri = self->m_rooms.begin();
		 ri != self->m_rooms.end(); ++ri) {
		// Check occupancy
		vacant_bed_count += (ri->m_capacity - ri->m_occupied);
	}

	return vacant_bed_count;
}

void Hotel_avail(Hotel *self, int *rooms, int *beds)
{
	*rooms = Hotel_availRooms(self);
	*beds = Hotel_availBeds(self);
}

Room *Room_new(RoomType level)
{
	return Room_newe(level, 0);
}

Room *Room_newe(RoomType level, int cap)
{
	return Room_newex(level, cap, false);
}

Room *Room_newex(RoomType level, int cap, bool pool)
{
	Room *nr = new Room;
	nr->m_level = level;
	nr->m_capacity = cap;
	nr->m_poolside = pool;
	nr->m_occupied = 0;
	return nr;
}

RoomType Room_getLevel(Room *room)
{
	return room->m_level;
}

Location *Location_new(std::string& country, std::string& city)
{
	Location *loc = new Location;
	loc->country = country;
	loc->city = city;
	return loc;
}

void Location_print(Location *self)
{
	std::cout << "(Location \"" << self->country << "\", \""
			  << self->city << "\")";
}

namespace {
	std::vector<Hotel> corporation;
}

void global_flush()
{
	corporation.resize(0);
}

void global_buy(Hotel& hotel)
{
	corporation.push_back(hotel);
}

void global_list()
{
	printf("%-20s %s\n", "HOTEL", "LOCATION");

	for (std::vector<Hotel>::iterator hotel_it = corporation.begin();
		 hotel_it != corporation.end(); ++hotel_it) {
		printf("%-20s %s, %s\n", hotel_it->m_name.c_str(),
			   hotel_it->m_location.city.c_str(),
			   hotel_it->m_location.country.c_str());
	}
}

void global_listex(char ***out)
{
	*out = new char *[corporation.size()];

	int index = 0;

	for (std::vector<Hotel>::iterator hotel_it = corporation.begin();
		 hotel_it != corporation.end(); ++hotel_it) {
		(*out)[index++] = strdup(hotel_it->m_name.c_str());
	}
}

class NoSuchHotelException : public std::exception 
{
public:
	const char *what() const throw () { return "hotel not in enterprise."; }
};

Hotel& global_findHotel(std::string name)
{
	for (std::vector<Hotel>::iterator hotel_it = corporation.begin();
		 hotel_it != corporation.end(); ++hotel_it) {
		// See
		if (hotel_it->m_name == name) return *hotel_it;
	}

	throw NoSuchHotelException();
}
