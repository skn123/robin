// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par PACKAGE
 * Pattern
 *
 * Implementation of basic_attached_filebuf methods.
 */

// System includes

#include <robin/debug/assert.h>
#include <errno.h>
#include <stdexcept>

// Local includes

// Package includes

namespace Pattern {

//-------------------------------------------------------------------
// @METHOD: basic_attached_filebuf
// @DESCRIPTION: Constructor;
//   1. Allocate the buffer,
//   2. Set the position pointers,
//   3. Store a reference to the file object.
//-------------------------------------------------------------------
template < class charT, class traits >
basic_attached_filebuf<charT, traits>::
basic_attached_filebuf(FILE *file, ios_base::openmode om, int buffer_size)
	:  m_mode(om), m_buffer(0),
	  m_buffer_size(buffer_size),m_file(file),  m_ended(false)
{
	assert(file != 0);
}

//-------------------------------------------------------------------
// @METHOD: basic_attached_filebuf
// @DESCRIPTION:
//-------------------------------------------------------------------
template < class charT, class traits >
basic_attached_filebuf<charT, traits>::~basic_attached_filebuf() {
	if (m_mode & ios_base::out) {
		flush_buffer();
	}
	delete[] m_buffer;
}

template < class charT, class traits >
void basic_attached_filebuf<charT, traits>::force_buffer_size(int buffer_size)
{
	m_buffer_size = buffer_size;
}

template < class charT, class traits >
void basic_attached_filebuf<charT, traits>::allocate_new_buffer()
{
	if (m_buffer) delete[] m_buffer;

    m_buffer = new char_type[m_buffer_size];
	// The buffer is initialized to be empty,
	// so the next read/write command will synchronize it.
	setg(m_buffer, m_buffer, m_buffer);
	setp(m_buffer, m_buffer);
}

//-------------------------------------------------------------------
// @METHOD: setbuf
// @DESCRIPTION: Default implementation - returns this.
//-------------------------------------------------------------------
template < class charT, class traits >
basic_streambuf<charT, traits>*  basic_attached_filebuf<charT, traits>::setbuf
	(typename basic_attached_filebuf<charT, traits>::char_type* n, streamsize s)
{
    return this;
}

//-------------------------------------------------------------------
// @METHOD: underflow
// @DESCRIPTION: Called when read buffer is empty.
//  If in input mode, refill the buffer using the refill_buffer()
//  method; otherwise, or if refill failed (due to EOF), return eof.
//-------------------------------------------------------------------
template < class charT, class traits >
typename basic_attached_filebuf<charT, traits>::
int_type basic_attached_filebuf<charT, traits>::underflow() {

    if (!m_ended && (m_mode & ios_base::in) && refill_buffer()) {
		return (unsigned char)*this->gptr();
	}
	else {
		m_ended = true;
		return traits::eof();
	}
}

//-------------------------------------------------------------------
// @METHOD: overflow
// @DESCRIPTION: Called when the write buffer fills up.
//  If in output mode, flush the buffer using the flush_buffer()
//  method; otherwise, return eof.
//-------------------------------------------------------------------
template < class charT, class traits >
typename basic_attached_filebuf<charT, traits>::
int_type basic_attached_filebuf<charT, traits>::overflow(typename basic_attached_filebuf<charT, traits>::int_type c) {
	if (m_mode & ios_base::out) {
		flush_buffer();
		return sputc(c);
	}
	else {
		return traits::eof();
	}
}

//-------------------------------------------------------------------
// @METHOD: pbackfail
// @DESCRIPTION:
//-------------------------------------------------------------------
template < class charT, class traits >
typename basic_attached_filebuf<charT, traits>::
int_type basic_attached_filebuf<charT,traits>::pbackfail(typename basic_attached_filebuf<charT, traits>::int_type c) {
	
	return traits::eof();
}


//-------------------------------------------------------------------
// @METHOD: refill_buffer
// @DESCRIPTION: Reads more characters from the Python file into the
//  buffer.
//-------------------------------------------------------------------
template < class charT, class traits >
bool basic_attached_filebuf<charT, traits>::refill_buffer() {

	if (m_buffer == NULL) allocate_new_buffer();
	if (this->gptr() < this->egptr())
		return true; /* there is still some data there */

	// Read a string from the file object
	char read_buffer[DEFAULT_BUFFER_SIZE];
	int read_len;

	read_len = fread(read_buffer, 1, m_buffer_size, m_file);

	if (read_len < 0)
		throw std::runtime_error(strerror(errno));

	// Notify eof if string is empty
	if (read_len == 0)
		return false;

	// Copy string contents to the buffer
	char *rstring = read_buffer;
	char *rend = rstring + read_len;
	char_type *edge;
	
	for (edge = m_buffer; rstring != rend; ++rstring, ++edge) {
		*edge = *rstring;  /* Data is assigned character by character to
							  allow possible conversion. */
	}

	// Update buffer pointers
	setg(m_buffer, m_buffer, edge);
	return true;
}

//-------------------------------------------------------------------
// @METHOD: flush_buffer
// @DESCRIPTION: Writes the contents of the buffer to the Python
//  file.
// The portion of the buffer written to the file is [pbegin,pnext).
// After the flush, pbegin <- pnext; but if as a result of that,
// the buffer becomes zero-lengthed (pbegin==pend), reallocate the
// buffer with max_buffer_size elements.
//-------------------------------------------------------------------
template < class charT, class traits >
void basic_attached_filebuf<charT,traits>::flush_buffer() {
	// TODO
#if 0
    // Create a char string from the buffer
	char_type *rnext = this->pbase();
	char_type *rend = this->pptr();
	
	std::string s;

	for (; rnext != rend; ++rnext) {
		s += (*rnext);
	}
	
	// Write the string to the Python file
	PyObjectHandle write_source = PyString_FromStringAndSize(s.c_str(), s.size());
	PyObjectHandle write_result = PyObject_CallMethod(m_pyfile, "write", "O",
													  (PyObject *)write_source);

	if (!write_result) {
		throw PythonException();
	}

	// Set the write pointers to a valid position
	if (this->pptr() < this->epptr()) {
		setp(this->pptr(), this->epptr());
	}
	else {
		// Reallocate with maximum size
		setp(m_buffer, m_buffer + max_buffer_size);
	}
#endif
}

//-------------------------------------------------------------------
// @METHOD: seekoff
// @DESCRIPTION: Empty implementation - seeking is not supported.
//-------------------------------------------------------------------
template < class charT, class traits > 
typename basic_attached_filebuf<charT, traits>::
pos_type basic_attached_filebuf<charT, traits>::
seekoff(typename basic_attached_filebuf<charT, traits>::off_type off,
		ios_base::seekdir way, ios_base::openmode which)
{
	// Cannot seek
	return pos_type(-1);
}

//-------------------------------------------------------------------
// @METHOD: seekoff
// @DESCRIPTION: Empty implementation - seeking is not supported.
//-------------------------------------------------------------------
template < class charT, class traits > 
typename basic_attached_filebuf<charT, traits>::pos_type
basic_attached_filebuf<charT, traits>::
seekpos(typename basic_attached_filebuf<charT, traits>::pos_type sp,
		ios_base::openmode which)
{
	// Cannot seek I tell ya!
	return pos_type(-1);
}


} // end of namespace Pattern
