// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/insight
 *
 * @par TITLE
 * Value Insight
 *
 * @par PACKAGE
 * Robin
 *
 * The Insight union is useful for acquiring more information about a value
 * than merely its type.
 */

#ifndef ROBIN_REFLECTION_INSIGHT_H
#define ROBIN_REFLECTION_INSIGHT_H


namespace Robin {


/**
 * A value insight.
 *
 * @par Contents
 * You can represent an insight either using a unique value for each category
 * or by memory addresses which represent different kinds - anyway, note that
 * the addresses themselves are compared and never the contents.
 *
 * @par Use
 * The insight is used solely to pass more information to conversion routines.
 *
 * @see Conversion, ConversionTable, Frontend
 */
union Insight
{
	void *i_ptr;
	long i_long;
};


inline bool operator==(Insight a, Insight b) { return a.i_ptr == b.i_ptr; }
inline bool operator!=(Insight a, Insight b) { return a.i_ptr != b.i_ptr; }
inline bool operator< (Insight a, Insight b) { return a.i_ptr < b.i_ptr; }
inline bool operator> (Insight a, Insight b) { return a.i_ptr > b.i_ptr; }


} // end of namespace Robin

#endif
