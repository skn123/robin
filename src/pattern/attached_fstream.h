// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 */

#ifndef PATTERN_ATTACHED_FSTREAM_H
#define PATTERN_ATTACHED_FSTREAM_H

// STL includes

#include <iostream>



// stl types
using std::basic_streambuf;
using std::ios_base;
using std::basic_istream;
using std::basic_ostream;
using std::streamsize;


namespace Pattern {

//====================================================================
// @CLASS: basic_attached_filebuf @basic_attached_filebuf@
// @DESCRIPTION: Implements a stream buffer associated with a C
//  FILE object.
// A small memory buffer (of size max_buffer_size) is held; when
// this buffer is emptied up while reading, it is refilled using the
// read() method of the file object. When it is filled up during
// writing, it is flushed and transferred to the write() method.
//====================================================================
template < class charT, class traits >
class basic_attached_filebuf : public basic_streambuf<charT, traits> {
public:
	basic_attached_filebuf(FILE *cfile, ios_base::openmode,
						   int buffer_size = DEFAULT_BUFFER_SIZE);
	virtual ~basic_attached_filebuf();
	
	typedef typename basic_streambuf<charT, traits>::int_type int_type;
	typedef typename basic_streambuf<charT, traits>::pos_type pos_type;
	typedef typename basic_streambuf<charT, traits>::char_type char_type;
	typedef typename basic_streambuf<charT, traits>::off_type off_type;

	void force_buffer_size(int buffer_size);

protected:
	//
    // 27.8.1.4 Overridden virtual functions -
	//  determine the nature of the stream buffer.
	// ************************************************** --->
	//
    virtual int_type underflow();
    virtual int_type overflow(int_type c=traits::eof());
    virtual int_type pbackfail(int_type c = traits::eof());

    virtual basic_streambuf<charT, traits>* setbuf(char_type* s, streamsize n);
    virtual pos_type seekoff(off_type off, ios_base::seekdir way, ios_base::openmode which=ios_base::in|ios_base::out);
    virtual pos_type seekpos(pos_type sp, ios_base::openmode which= ios_base::in|ios_base::out);

    //virtual int sync();  /* Locale methods; optional. */
    //virtual void imbue(const locale& loc);

	// <--- **************************************************
	//

	void allocate_new_buffer();

	bool refill_buffer();
	void flush_buffer();
	
private:
	ios_base::openmode  m_mode;
	char_type          *m_buffer;
	int                 m_buffer_size;
	FILE               *m_file;
	bool                m_ended;
	
	static const int DEFAULT_BUFFER_SIZE;
};	

template < class charT, class traits >
const int basic_attached_filebuf<charT,traits>::DEFAULT_BUFFER_SIZE = 96;

//====================================================================
// @CLASS: basic_attached_ifstream
// @DESCRIPTION: An input stream, connected to a attached_filebuf.
//====================================================================
template < class charT, class traits >
class basic_attached_ifstream : public basic_istream<charT, traits> {
public:
    typedef charT                     char_type;
    typedef typename traits::pos_type pos_type;
    typedef typename traits::off_type off_type;
    typedef typename traits::int_type int_type;
    typedef traits                    traits_type;

    typedef basic_attached_filebuf<char_type, traits> buf_type;

    explicit basic_attached_ifstream(FILE *withfile)
		: sbuf(withfile, ios_base::in), 
		  basic_istream<charT, traits>(&sbuf)
	{
    	init(&sbuf);
    }
    explicit basic_attached_ifstream(FILE *withfile, int buffer_size)
		: basic_istream<charT, traits>(&sbuf),
		sbuf(withfile, ios_base::in, buffer_size)
	{
    	init(&sbuf);
    }
    virtual ~basic_attached_ifstream() { }

    buf_type* rdbuf() const { return &sbuf; }

private:
	buf_type sbuf;
};

//====================================================================
// @CLASS: basic_attached_ofstream @basic_attached_ofstream@
// @DESCRIPTION: An output stream, connected to a attached_filebuf.
//====================================================================
template < class charT, class traits >
class basic_attached_ofstream : public basic_ostream<charT, traits> {
public:
    typedef charT                     char_type;
    typedef typename traits::pos_type pos_type;
    typedef typename traits::off_type off_type;
    typedef typename traits::int_type int_type;
    typedef traits                    traits_type;

    typedef basic_attached_filebuf<char_type, traits> buf_type;


	explicit basic_attached_ofstream(FILE *withfile)
		: sbuf(withfile, ios_base::out), basic_ostream<charT, traits>(&sbuf) { init(&sbuf); }
	virtual ~basic_attached_ofstream() { }
	
    buf_type* rdbuf() const { return &sbuf; }

protected:

private:
	buf_type sbuf;
};	



typedef basic_attached_ifstream<char, std::char_traits<char> >
	attached_ifstream;
typedef basic_attached_ofstream<char, std::char_traits<char> > 
	attached_ofstream;

} // end of namespace Pattern


#include "attached_fstream.inl"

#endif
