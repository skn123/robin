// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @par SOURCE
 * extreme/test_reflection_arena.h
 *
 * @par TITLE
 * Reflection Test - Sample Library
 *
 * @par PACKAGE
 * Robin
 *
 * Based on the "Enterprise" library, this section provides
 * exposure to programming elements contained in our sample enterprise,
 * through Robin's reflection mechanism.
 */

#ifndef ROBIN_REFLECTION_TEST_ARENA_HOTEL_ENTERPRISE_H
#define ROBIN_REFLECTION_TEST_ARENA_HOTEL_ENTERPRISE_H

#include "enterprise.h"

#include <pattern/handle.h>
#include <robin/reflection/class.h>
#include <robin/reflection/enumeratedtype.h>


namespace Robin { class Namespace; }
Handle<Robin::Namespace> registerEnterprise();

extern Handle<Robin::Class> enterprise_string;
extern Handle<Robin::Class> enterprise_Location;
extern Handle<Robin::Class> enterprise_Room;
extern Handle<Robin::Class> enterprise_Hotel;
extern Handle<Robin::EnumeratedType> enterprise_RoomType;


#endif
