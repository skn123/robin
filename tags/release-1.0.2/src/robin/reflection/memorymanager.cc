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

#include "memorymanager.h"
#include "../frontends/framework.h"
#include "../frontends/frontend.h"


namespace Robin {

/**
 * Creates another reference to an existing scripting element by invoking
 * the appropriate method of the currently active frontend.
 */
scripting_element MemoryManager::duplicateReference(scripting_element element)
{
	return FrontendsFramework::activeFrontend()->duplicateReference(element);
}

/**
 * Frees an associated scripting element by invoking the 
 * approperiate method of the currently active frontend.
 */
void MemoryManager::release(scripting_element element)
{
	FrontendsFramework::activeFrontend()->release(element);
}


/**
 * Report that an element should be destroyed during this session of
 * garbage collection.
 */
void GarbageCollection::markForDestruction(scripting_element element)
{
	m_heap[m_size++] = element;
}

/**
 * Destroys all the objects in the current garbage heap and empties it.
 */
void GarbageCollection::cleanUp()
{
	while (m_size > 0)
		MemoryManager::release(m_heap[--m_size]);
}



} // end of namespace Robin
