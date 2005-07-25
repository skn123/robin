#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>

struct RegData { const char *name; const char *type; RegData *i; void *sym; };
struct PascalString { unsigned long size; const char *chars; char buffer[1]; };

#define F (void*)&

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
	{ 0 }
};

RegData stdifstream_proto[] = {
	{ "std::istream", "extends", 0, F upcast_stdifstream_to_stdistream },
	{ "*", "constructor", ctor_stdifstream1_proto, F ctor_stdifstream1 },
	{ "good", "bool", 0, F stdifstream_good },
	{ ".", "destructor", 0, F dtor_stdifstream },
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
	{ "std::ifstream", "class", stdifstream_proto, 0 },
	{ 0 }
};
