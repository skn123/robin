/* -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file
 *
 * @par SOURCE
 * registration/regdata.cc
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

#include "regdata.h"

#include <stdio.h>


namespace { const int DUMP_BASIC_OFFSET = 3; }

namespace Robin {





/**
 * Dumps the contents of the entire structure, which
 * is assumed to be rooted here.
 */
void RegData::dbgout() const
{
	dump(0);
}

/**
 * Recursively scans arrays of RegData-s, printing out
 * their contents to the standard error. When a structure refers to
 * another set, they are also dumped with increased indent.
 */
void RegData::dump(int indent) const
{
	for (const RegData *pdata = this; pdata->name != NULL; ++pdata) {
		fprintf(stderr, "// %*s + %s (%s)\n", indent, "", 
				pdata->name, pdata->type);
		// Recursively process referred structure
		if (pdata->prototype)
			pdata->prototype->dump(indent + DUMP_BASIC_OFFSET);
	}
}

} // end of namespace Robin
