/* -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

#include <stdio.h>
#include <string.h>

#include "fwtesting.h"
#include <robin/reflection/library.h>
#include <robin/frontends/framework.h>
#include <robin/frontends/simple/simplefrontend.h>
#include <robin/registration/mechanism.h>

#include <robin/debug/trace.h>

#include "interactive/syntax.h"


Handle<Robin::Library> enterprise;

/**
 */
int main(int argc, char *argv[])
{
	if (argc > 1 && argv[1][0] == '+')
		dbg::trace.enable();

    Handle<Robin::Frontend> fe(new Robin::SimpleFrontend);

    Robin::FrontendsFramework::selectFrontend(fe);

	// Register the enterprise library
	try {
		enterprise = Robin::RegistrationMechanismSingleton
			::getInstance()->import("libenterprise.so");
	}
	catch (Robin::DynamicLibraryOpenException& e) {
		std::cerr << "// @FATAL: while importing enterprise: "
				  << e.what() << std::endl;
		std::cerr << "// @DLERROR: " << e.dlerror_at << std::endl;
		exit(1);
	}
	catch (std::exception& e) {
		std::cerr << "// @FATAL: while importing enterprise: "
				  << e.what() << std::endl;
		exit(1);
	}

	Extreme::TestingProgram::engage();
	return 0;
}

