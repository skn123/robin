// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/framework.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "framework.h"

#include "../reflection/intrinsic_type_arguments.h"
#include "../reflection/class.h"


namespace Robin {



Handle<Frontend> FrontendsFramework::active_frontend;



/**
 * Sets the specified frontend as the active frontend.
 * This method initializes and deploys the transferred frontend, and
 * keeps a reference to it so subsequent calls to activeFrontend()
 * will return it for use in translation (for example, this is heavily
 * used by <classref>OverloadedSet</classref> mechanisms.
 */
void FrontendsFramework::selectFrontend(Handle<Frontend> fe)
{
	active_frontend = fe;
	active_frontend->initialize();
	// Request adapters for all the intrinsic types
	fillAdapter(Robin::ArgumentInt);
	fillAdapter(Robin::ArgumentLong);
	fillAdapter(Robin::ArgumentLongLong);
	fillAdapter(Robin::ArgumentUInt);
	fillAdapter(Robin::ArgumentULong);
	fillAdapter(Robin::ArgumentULongLong);
	fillAdapter(Robin::ArgumentShort);
	fillAdapter(Robin::ArgumentUShort);
	fillAdapter(Robin::ArgumentChar);
	fillAdapter(Robin::ArgumentUChar);
	fillAdapter(Robin::ArgumentFloat);
	fillAdapter(Robin::ArgumentDouble);
	fillAdapter(Robin::ArgumentBoolean);
	fillAdapter(Robin::ArgumentCString);
	fillAdapter(Robin::ArgumentPascalString);
	fillAdapter(Robin::ArgumentScriptingElementNewRef);
    fillAdapter(Robin::ArgumentScriptingElementBorrowedRef);
}

/**
 * Returns the frontend that was set by the 
 * <methodref>selectFrontend()</methodref> method.
 *
 * @throws EnvironmentVacuumException if there is no active frontend.
 */
Frontend *FrontendsFramework::activeFrontend()
{
	if (active_frontend)
		return &*active_frontend;
	else
		throw EnvironmentVacuumException();
}

/**
 * (internal) supplies an adapter to the specified
 * <classref>RobinType</classref> using the active frontend.
 */
void FrontendsFramework::fillAdapter(Handle<RobinType> toa)
{
	if (toa) {
		try {
			toa->assignAdapter(active_frontend->giveAdapterFor(*toa));
		}
		catch (UnsupportedInterfaceException& ) {
			/* Interface cannot be filled */
		}
	}
}

/**
 * (internal) supplies adapters to both the PtrArg and
 * RefArg of a class.
 * <p>The registration mechanism uses this function.</p>
 */
void FrontendsFramework::fillAdapters(Handle<Class> cls)
{
	fillAdapter(cls->getConstType());
	fillAdapter(cls->getPtrType());
	fillAdapter(cls->getType());
}


} // end of namespace Robin
