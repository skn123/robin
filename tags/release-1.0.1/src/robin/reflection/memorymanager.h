// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/memorymanager.h
 *
 * @par TITLE
 * Memory Management Utilities for Reflection Methods
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_REFLECTION_MEMORYMANAGER_H
#define ROBIN_REFLECTION_MEMORYMANAGER_H

#include "callable.h"


namespace Robin {

/**
 * Provides basic memory management routines.
 */
class MemoryManager
{
public:
	static scripting_element duplicateReference(scripting_element element);
	static void release(scripting_element element);
};

/**
 * Holds a session of a very simple garbage collection layer. When the
 * reflection detects an element that is no longer used, it is added to the
 * garbage heap. Later, the reflection collects and cleans all these objects
 * up.
 */
class GarbageCollection
{
public:
	inline GarbageCollection() : m_size(0) { }

	void markForDestruction(scripting_element element);
	void cleanUp();

private:
	static const int GARBAGE_HEAP_SIZE = 12;
	
	scripting_element m_heap[GARBAGE_HEAP_SIZE];
	int m_size;
};


} // end of namespace Robin

#endif

