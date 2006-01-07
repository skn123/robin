// -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @par TITLE
 * Robin Runtime Benchmark
 *
 * @par PACKAGE
 * Robin
 *
 * A short program which invokes Robin's execution runtime in a tight
 * loop. This program can be used for profiling in order to find run-time
 * bottlenecks.
 */

#include "../test_reflection_arena.h"
#include <robin/reflection/namespace.h>
#include <robin/reflection/instance.h>
#include <robin/frontends/simple/elements.h>
#include <robin/frontends/simple/simplefrontend.h>
#include <robin/frontends/framework.h>

#include <robin/debug/trace.h>

const unsigned long DEFAULT_NUMBER_OF_TIMES = 1000000;


void tightLoop(int times)
{
	// Some sample values
	Simple::String *s_france = new Simple::String;

	s_france->value = "France";

	// Begin tight loop
	while (--times > 0) {
		Robin::ActualArgumentList args;
		args.push_back((Robin::scripting_element)(Simple::Element*)s_france);
		enterprise_string->createInstance(args);
	}
}

int main(int argc, char *argv[])
{
    Handle<Robin::Frontend> fe(new Robin::SimpleFrontend);

    Robin::FrontendsFramework::selectFrontend(fe);
    registerEnterprise();

    // A '+' as a command line argument enables debug tracing
    if (argc >= 2 && argv[1][0] == '+') {
		dbg::trace.enable();
		--argc; ++argv; /* shift */
    }

	try {
		tightLoop((argc < 2) ? DEFAULT_NUMBER_OF_TIMES : 1 << atoi(argv[1]));
	}
	catch (std::exception& e) {
		std::cerr << "*** Ouch! Runtime exception: "
				  << e.what() << std::endl;
	}
}
