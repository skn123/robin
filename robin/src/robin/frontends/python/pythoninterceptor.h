// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 */

#ifndef ROBIN_FRONTENDS_PYTHON_INTERCEPTOR_H
#define ROBIN_FRONTENDS_PYTHON_INTERCEPTOR_H


#include "../../reflection/interface.h"
#include "../../frontends/framework.h"
#include "../../frontends/frontend.h"


namespace Robin {

namespace Python {


class PythonInterceptor : public Interceptor
{
public:
	virtual ~PythonInterceptor();

	virtual basic_block callback(scripting_element twin, 
								 const Signature& signature,
								 basic_block args[]) const;

private:
	void reportCallbackError() const;
};


} // end of namespace Robin::Python

} // end of namespace Robin


#endif
