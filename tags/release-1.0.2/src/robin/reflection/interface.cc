// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

#include "interface.h"

#include <assert.h>

#include "../frontends/framework.h"
#include "../frontends/frontend.h"
#include "../registration/regdata.h"
#include "../debug/trace.h"


namespace Robin {


/**
 * Triggers callback() on the active frontend's interceptor.
 */
basic_block Interface::global_callback(scripting_element twin,
								RegData *signature,
								basic_block args[])
{
	Signature *real_signature = reinterpret_cast<Signature*>(signature->sym);

	assert(real_signature);

	dbg::trace << "// Interface::global_callback "
			   << (void*)real_signature << " '" << real_signature->name
			   << "' " << twin << dbg::endl;

	return FrontendsFramework::activeFrontend()
		->getInterceptor().callback(twin, *real_signature, args);
}


} // end of namespace Robin
