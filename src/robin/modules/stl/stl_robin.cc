#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <stdio.h>

#include "streams.h"

struct RegData { const char *name; const char *type; RegData *i; void *sym; };
struct PascalString { unsigned long size; const char *chars; char buffer[1]; };
typedef void *scripting_element;
typedef void *basic_block;

#define F (void*)&

typedef bool (*__interceptor)(scripting_element twin,
			      RegData *signature, basic_block args[], 
			      basic_block *result, bool isPure);
extern __interceptor __robin_callback;
__interceptor __robin_callback = 0;

/*
 * std::string
 */

std::string *ctor_stdstring0()
{
	return new std::string;
}

std::string *ctor_stdstring1(const char *cstr)
{
	return new std::string(cstr);
}
RegData ctor_stdstring1_proto[] = {
	{ "cstr", "*char", 0, 0 },
	{ 0 }
};

std::string *ctor_stdstring2(const char *data, unsigned long size)
{
	return new std::string(data, size);
}

RegData ctor_stdstring2_proto[] = {
	{ "data", "*char", 0, 0 },
	{ "size", "unsigned long", 0, 0 },
	{ 0 }
};

std::string *ctor_stdstringpascal(unsigned long size, const char *chars)
{
	return new std::string(chars, size);
}
RegData ctor_stdstringpascal_proto[] = {
	{"pascal", "@string", 0, 0 },
	{ 0 }
};

PascalString *stdstring_toString(std::string *self)
{
	PascalString *pascal = (PascalString*)malloc(sizeof(PascalString));
	pascal->size = (unsigned long)(self->size());
	pascal->chars = self->data();
	return pascal;
}

unsigned long stdstring_size(std::string& self)
{
	return self.size();
}

void stdstring_dtor(std::string *self)
{
	delete self;
}

/*
 * std::istream
 */
std::istream *ctor_stdistream(std::streambuf *buf)
{
	return new std::istream(buf);
}

RegData ctor_stdistream_proto[] = {
	{ "buf", "*Igluebuf", 0, 0 },
	{ 0 }
};

/*
 * std::ostream
 */
std::ostream *ctor_stdostream(std::streambuf *buf)
{
	return new std::ostream(buf);
}

RegData ctor_stdostream_proto[] = {
	{ "buf", "*Igluebuf", 0, 0 },
	{ 0 }
};

/*
 * std::ostringstream
 */

std::ostream *upcast_stdostringstream_to_stdostream(std::ostringstream& o)
{
	return &o;
}

std::ostringstream *ctor_stdostringstream0()
{
	return new std::ostringstream;
}

std::string *stdostringstream_str(const std::ostringstream& self)
{
	return new std::string(self.str());
}

void stdostringstream_dtor(std::ostringstream *self)
{
	delete self; 
}

/*
 * std::ofstream
 */

std::ostream *upcast_stdofstream_to_stdostream(std::ofstream& o)
{
	return &o;
}

std::ofstream *ctor_stdofstream1(const char *filename)
{
	return new std::ofstream(filename);
}

RegData ctor_stdofstream1_proto[] = {
	{ "filename", "*char", 0, 0 },
	{ 0 }
};

bool stdofstream_good(std::ofstream *self)
{
	return self->good();
}

void dtor_stdofstream(std::ofstream *self)
{
	delete self;
}

/*
 * std::ifstream
 */

std::istream *upcast_stdifstream_to_stdistream(std::ifstream& i)
{
	return &i;
}

std::ifstream *ctor_stdifstream1(const char *filename)
{
	return new std::ifstream(filename);
}

RegData ctor_stdifstream1_proto[] = {
	{ "filename", "*char", 0, 0 },
	{ 0 }
};

bool stdifstream_good(std::ifstream *self)
{
	return self->good();
}

void dtor_stdifstream(std::ifstream *self)
{
	delete self;
}

/*
 * std::istringstream
 */

std::istream *upcast_stdistringstream_to_stdistream(std::istringstream& i)
{
	return &i;
}

std::istringstream *ctor_stdistringstream1(const std::string& buffer)
{
	return new std::istringstream(buffer);
}

RegData ctor_stdistringstream1_proto[] = {
	{ "buffer", "&std::string", 0, 0 },
	{ 0 }
};

void dtor_stdistringstream(std::istringstream *self)
{
	delete self;
}

/*
 * std::streambuf
 */
extern RegData Igluebuf_proto[];

class Igluebuf : public ::gluebuf
{
public:
	void _init(scripting_element imp) { twin = imp; }

	virtual void write(const std::string &data) {
		void *args[] = { (void*)(&data) };
		void *result;
		__robin_callback(twin, Igluebuf_proto+2, args, &result, true);
	}

	virtual std::string read(int n) { 
		void *args[] = { (void*)n };
		void *result;
		__robin_callback(twin, Igluebuf_proto+3, args, &result, true);
		return *std::auto_ptr<std::string>((std::string*)result);
	}

private:
	scripting_element twin;
};

Igluebuf *ctor_Igluebuf() { return new Igluebuf; }
void dtor_Igluebuf(Igluebuf *esc) { delete esc; };

void Igluebuf_init(Igluebuf *gb, scripting_element imp) { gb->_init(imp); }

RegData Igluebuf_init_proto[] = {
	{ "imp", "scripting_element", 0, 0 },
	{ 0 }
};

RegData Igluebuf_write_proto[] = {
	{ "data", "&std::string", 0, 0 },
	{ 0 }
};

RegData Igluebuf_read_proto[] = {
	{ "n", "int", 0, 0 },
	{ 0 }
};


RegData stdstring_proto[] = {
	{ "*", "constructor", 0, F ctor_stdstring0 },
	//{ "*", "constructor", ctor_stdstring1_proto, F ctor_stdstring1 },
	{ "*", "constructor", ctor_stdstring2_proto, F ctor_stdstring2 },
	{ "^", "constructor", ctor_stdstringpascal_proto, F ctor_stdstringpascal },
	{ "size", "unsigned long", 0, F stdstring_size },
	{ ".string", "@string", 0, F stdstring_toString },
	{ ".", "destructor", 0, F stdstring_dtor },
	{ 0 }
};

RegData stdostream_proto[] = {
	{ "*", "constructor", ctor_stdostream_proto, F ctor_stdostream },
	{ 0 }
};

RegData stdostringstream_proto[] = {
	{ "std::ostream", "extends", 0, F upcast_stdostringstream_to_stdostream },
	{ "*", "constructor", 0, F ctor_stdostringstream0 },
	{ "str", "*std::string", 0, F stdostringstream_str },
	{ ".", "destructor", 0, F stdostringstream_dtor },
	{ 0 }
};

RegData stdofstream_proto[] = {
	{ "std::ostream", "extends", 0, F upcast_stdofstream_to_stdostream },
	{ "*", "constructor", ctor_stdofstream1_proto, F ctor_stdofstream1 },
	{ "good", "bool", 0, F stdofstream_good },
	{ ".", "destructor", 0, F dtor_stdofstream },
	{ 0 }
};

RegData stdistream_proto[] = {
	{ "*", "constructor", ctor_stdistream_proto, F ctor_stdistream },
	{ 0 }
};

RegData stdifstream_proto[] = {
	{ "std::istream", "extends", 0, F upcast_stdifstream_to_stdistream },
	{ "*", "constructor", ctor_stdifstream1_proto, F ctor_stdifstream1 },
	{ "good", "bool", 0, F stdifstream_good },
	{ ".", "destructor", 0, F dtor_stdifstream },
	{ 0 }
};

RegData stdistringstream_proto[] = {
	{ "std::istream", "extends", 0, F upcast_stdistringstream_to_stdistream },
	{ "*", "constructor", ctor_stdistringstream1_proto, F ctor_stdistringstream1 },
	{ ".", "destructor", 0, F dtor_stdistringstream },
	{ 0 }
};

RegData Igluebuf_proto[] = {
	{ "*", "constructor", 0, F ctor_Igluebuf },
	{ "_init", "void", Igluebuf_init_proto, F Igluebuf_init },
	{ "write", "void", Igluebuf_write_proto, 0 },
	{ "read", "*std::string", Igluebuf_read_proto, 0 },
	{ ".", "destructor", 0, F dtor_Igluebuf },
	{ 0 }
};

#ifdef _WIN32
extern "C"
__declspec(dllexport)
#endif
RegData entry[] = {
	{ "std::string", "class", stdstring_proto, 0 },
	{ "std::ostream", "class", stdostream_proto, 0 },
	{ "std::ostringstream", "class", stdostringstream_proto, 0 },
	{ "std::ofstream", "class", stdofstream_proto, 0 },
	{ "std::istream", "class", stdistream_proto, 0 },
	{ "std::istringstream", "class", stdistringstream_proto, 0 },
	{ "std::ifstream", "class", stdifstream_proto, 0 },
	{ "Igluebuf", "class", Igluebuf_proto, 0 },
	{ 0 }
};
