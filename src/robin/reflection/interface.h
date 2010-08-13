// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#ifndef ROBIN_REFLECTION_INTERFACE_H
#define ROBIN_REFLECTION_INTERFACE_H

#include <vector>
#include <string>

// Pattern includes
#include <pattern/handle.h>

// Robin includes
#include "robintype.h"


namespace Robin {


struct Signature
{
	std::string name;
	Handle<RobinType> returnType;
	std::vector< Handle<RobinType> > argumentTypes;
};

/**
 * Supports callbacks from C++ interfaces into scripting environment code.
 */
class Interceptor
{
public:
	virtual ~Interceptor() { }

	/**
	 * Invokes a scripting-environment function with the given signature and
	 * arguments.
	 *
	 * Returns true if the call succeeded, false if no appropriate function was found.
	 */
	virtual bool callback(scripting_element twin, const Signature& signature,
						  basic_block args[], basic_block& result) const = 0;

};


struct RegData;

class Interface
{
public:
	typedef bool (*callback_t)(scripting_element twin,
							   RegData *signature,
							   basic_block args[],
							   basic_block *result,
                               bool pureVirtual);

	static bool global_callback(scripting_element twin,
								RegData *signature,
								basic_block args[],
								basic_block *result,
                                bool pureVirtual);
};



} // end of namespace Robin



#endif
