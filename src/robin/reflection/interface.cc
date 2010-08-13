// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#include "interface.h"

#include <robin/debug/assert.h>
#include <stdexcept>

#include "../frontends/framework.h"
#include "../frontends/frontend.h"
#include "../registration/regdata.h"
#include "../debug/trace.h"


namespace Robin {


/**
 * Triggers callback() on the active frontend's interceptor.
 */
bool Interface::global_callback(scripting_element twin,
								RegData *signature,
								basic_block args[],
								basic_block *result,
                                bool pureVirtual)
{
	Signature *real_signature = reinterpret_cast<Signature*>(signature->sym);

	assert_true(real_signature);

	dbg::trace << "// Interface::global_callback "
			   << (void*)real_signature << " '" << real_signature->name
			   << "' " << twin << dbg::endl;

	bool success = FrontendsFramework::activeFrontend()
		->getInterceptor().callback(twin, *real_signature, args, *result);

    if (!success && pureVirtual) {
		throw std::runtime_error("pure virtual method '" + real_signature->name + 
								 "' is not implemented.");
    }

    return success;
}


} // end of namespace Robin
