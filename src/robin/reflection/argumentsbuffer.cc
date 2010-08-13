// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/argumentsbuffer.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "argumentsbuffer.h"

#include <robin/debug/assert.h>
#include <string.h>


namespace Robin {





/**
 * Empty constructor. Add arguments using push().
 */
ArgumentsBuffer::ArgumentsBuffer()
   : m_pend(m_buffer)
{
}



/**
 * Blindly copies data from a buffer and packs it
 * on the internal stack.
 */
void ArgumentsBuffer::push(void *data, int datasize)
{
	assert(false); // not implemented
}



/**
 * Pushes an 'int' value on the stack.
 */
void ArgumentsBuffer::pushInt(int value)
{
	(*m_pend++) = reinterpret_cast<basic_block>(value);
}

/**
 * Pushes a 'char' value on the stack.
 */
void ArgumentsBuffer::pushChar(char value)
{
	(*m_pend++) = reinterpret_cast<basic_block>(value);
}

/**
 * Pushes a 'long' value on the stack.
 */
void ArgumentsBuffer::pushLong(long value)
{
	(*m_pend++) = reinterpret_cast<basic_block>(value);
}

/**
 * Pushes a 'float' value of the stack.
 */
void ArgumentsBuffer::pushFloat(float value)
{
#define FLOAT_BLOCKS \
       (sizeof(float) + sizeof(basic_block) / 2) / sizeof(basic_block)

	union {
		float value;
		basic_block rep[FLOAT_BLOCKS];
	} f;
	f.value = value;
	memcpy(m_pend, f.rep, sizeof(f.rep));
	m_pend += FLOAT_BLOCKS;
}

/**
 * Pushes a 'void *' value on the stack.
 */
void ArgumentsBuffer::pushPointer(const void * value)
{
	(*m_pend++) = (basic_block)value;
}



/**
 * Returns the internal arguments buffer, for use in
 * low-level calls.
 */
const basic_block *ArgumentsBuffer::getBuffer() const
{
    return m_buffer;
}

} // end of namespace Robin

