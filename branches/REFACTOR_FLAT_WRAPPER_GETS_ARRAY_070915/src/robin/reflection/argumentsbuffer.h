// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * argumentsbuffer.h
 *
 * The ArgumentsBuffer class is the formal approach towards
 * passing arguments to external functions: the arguments are packed into
 * a tight array, which is then passed 'as-is'.<br />
 * The <classref>Adapter</classref> interface is constructed using this
 * concept.
 *
 * @par PUBLIC-CLASSES
 * <ul><li>ArgumentsBuffer</li></ul>
 */

#ifndef ROBIN_ARGUMENTSBUFFER_H
#define ROBIN_ARGUMENTSBUFFER_H

#include <cstddef>

namespace Robin {

typedef void *basic_block;   /* should be a machine word */

/**
 * \@CONSTANTS
 */

// the size of a buffer of arguments, in machine words
#define ARGUMENTSBUFFER_SIZE 40

/**
 * @class ArgumentsBuffer
 * @nosubgrouping
 *
 * Represents a 'stack' of arguments which are to be
 * passed to a function. The arguments are push()ed, that is, packed,
 * into an array of word-size aligned bytes. This array meets the
 * requirements of the ANSI-C calling convention and can thus be 
 * passed to an external function given by address.
 */
class ArgumentsBuffer
{
public:
	/**
	 * @name Constructors
	 */
	//@{
	ArgumentsBuffer();
    //@}

    /**
     * @name Pushing data
     */
    //@{
    void push(void *data, int datasize);
	//@}

    /**
     * @name Pushing data!Pushing specific data elements
     */
	//@{
    void pushInt(int value);
    void pushChar(char value);
    void pushLong(long value);
	void pushFloat(float value);
	void pushPointer(const void *value);

	inline void push(int value)         { pushInt(value); }
	inline void push(long value)        { pushLong(value); }
	inline void push(char value)        { pushChar(value); }
	inline void push(float value)       { pushFloat(value); }
	inline void push(const void *value) { pushPointer(value); }
    //@}
    
    size_t size() const { return m_pend - m_buffer; }

    /**
     * @name Access
     */
    //@{
    const basic_block *getBuffer() const;
	//@}

  private:
    basic_block m_buffer[ARGUMENTSBUFFER_SIZE];
    basic_block *m_pend;
};


} // end of namespace Robin


/**
 * Some syntactic sugaring
 */
inline Robin::ArgumentsBuffer& operator << (Robin::ArgumentsBuffer& ab,
					      int value)
    { ab.pushInt(value); return ab; }
inline Robin::ArgumentsBuffer& operator << (Robin::ArgumentsBuffer& ab,
					      char value)
    { ab.pushChar(value); return ab; }
inline Robin::ArgumentsBuffer& operator << (Robin::ArgumentsBuffer& ab,
					      long value)
    { ab.pushLong(value); return ab; }

#endif
