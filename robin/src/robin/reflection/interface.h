// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#ifndef ROBIN_REFLECTION_INTERFACE_H
#define ROBIN_REFLECTION_INTERFACE_H

#include <vector>
#include <string>

// Pattern includes
#include <pattern/handle.h>

// Robin includes
#include "typeofargument.h"


namespace Robin {


struct Signature
{
	std::string name;
	Handle<TypeOfArgument> returnType;
	std::vector< Handle<TypeOfArgument> > argumentTypes;
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
	 */
	virtual basic_block callback(scripting_element twin, const Signature& signature,
						  basic_block args[]) const = 0;

};


struct RegData;

class Interface
{
public:
	typedef basic_block (*callback_t)(scripting_element twin,
							   RegData *signature,
							   basic_block args[]);

	static basic_block global_callback(scripting_element twin,
								RegData *signature,
								basic_block args[]);
};



} // end of namespace Robin



#endif
