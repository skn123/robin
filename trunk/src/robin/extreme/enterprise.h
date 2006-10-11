// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @par SOURCE
 * extreme/enterprise.h
 *
 * @par TITLE
 * Testing Purposes Sample Library
 *
 * @par PACKAGE
 * Robin
 *
 * A sample set of classes, defined as mere structures
 * with flat functions manipulating them, is presented here for the
 * sole purpose of testing the reflection mechanism.
 * Because we strive to achieve high style, the sample brought here
 * models a world-wide hotel enterprise.
 */

#ifndef ROBIN_EXTREME_TEST_ARENA_HOTEL_ENTERPRISE_H
#define ROBIN_EXTREME_TEST_ARENA_HOTEL_ENTERPRISE_H

#include <string>
#include <list>
#include <vector>
#include <iostream>


struct Location
{
	std::string country;
	std::string city;
};

bool operator == (const Location&, const Location&);
std::ostream& operator << (std::ostream&, const Location&);

enum RoomType
{
	TOURIST,
	LEISURE,
	SUITE,
	EMBASSY
};

struct Room
{
	RoomType   m_level;
	int        m_capacity;
	bool       m_poolside;
	int        m_occupied;
};

bool operator == (const Room&, const Room&);
std::ostream& operator << (std::ostream&, const Room&);

struct Hotel
{
	std::string       m_name;
	Location          m_location;
	std::vector<Room> m_rooms;
	bool              m_hasPool;
};

// Method prototypes
std::string *string_new(const char *cstr);
Hotel *Hotel_new(std::string& name, Location& location);
Hotel *Hotel_copy(Hotel& hotel);
const char *Hotel_getName(Hotel *self);
Location& Hotel_getLocation(Hotel *self);
Room& Hotel_getRoomNo(Hotel *self, int room_number);
void Hotel_buildRoom(Hotel *self, Room& room);
void Hotel_buildPool(Hotel *self);
void Hotel_checkin(Hotel *self, RoomType level, int guests);
int Hotel_availRooms(Hotel *self);
int Hotel_availBeds(Hotel *self);
void Hotel_avail(Hotel *self, int *rooms, int *beds);
Room *Room_new(RoomType level);
Room *Room_newe(RoomType level, int capacity);
Room *Room_newex(RoomType level, int capacity, bool pool);
RoomType Room_getLevel(Room *room);
void Room_occupy(Room *self);
Location *Location_new(std::string& country, std::string& city);
void Location_print(Location *self);

void global_buy(Hotel& hotel);
void global_list();
Hotel& global_findHotel(std::string name);
void global_flush();

#endif
