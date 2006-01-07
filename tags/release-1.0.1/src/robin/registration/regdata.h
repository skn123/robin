/* -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file
 *
 * @par SOURCE
 * registration/regdata.h
 *
 * @par TITLE
 * Registration Data Structure
 *
 * @par PACKAGE
 * Robin
 *
 * @par PUBLIC-CLASSES
 *
 * <ul><li>RegData</li></ul>
 */

#ifndef ROBIN_REGISTRATION_DATA_H
#define ROBIN_REGISTRATION_DATA_H


namespace Robin {

/**
 * @class RegData
 * @nosubgrouping
 *
 * Holds the smallest element of the registration.
 * Combining such elements in a hierarchical manner can be used to
 * describe the interface of libraries. This structure is visible and
 * used as a protocol to the registration mechanism: each library
 * exports a symbol referring to an array of RegData, which may in
 * turn point to more arrays to describe functions and classes.
 */
struct RegData
{
	const char *name;
	const char *type;
	RegData *prototype;

	void *sym;

	// Used for debug trace
	void dbgout() const;
	void dump(int indent) const;
};

} // end of namespace Robin

#endif
