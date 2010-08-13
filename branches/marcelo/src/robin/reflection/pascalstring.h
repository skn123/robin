// -*- mode: c++; 

/**
 * @file
 *
 * @par SOURCE
 * pascalstring.h
 *
 * @par TITLE
 * Pascal String Auxiliary Data Structure
 *
 * Declares a data structure which provides convenient data flow for some
 * interfaces by filling a gap in the C programming language. It does not
 * mean that Robin has some intention of wrapping Pascal code, just to
 * steal a useful concept.
 */

#ifndef ROBIN_PASCALSTRING_H
#define ROBIN_PASCALSTRING_H

/**
 * Pascal-style string; this string keeps the length as its first data
 * item, characters follow.
 *
 * @par structure
 * The pointer 'chars' may be used to redirect string's data to an existing
 * C string without copying its contents; otherwise, 'chars' should point to
 * buffer[0]. The memory size allocated for the structure may exceed
 * <code>sizeof(PascalString)</code> in order to contain character data in
 * 'buffer'.
 */
struct PascalString
{
	unsigned long size;
	char *chars;
	char buffer[1];
};


#endif
