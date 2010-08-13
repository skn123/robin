// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

#include "fwtesting.h"
#include "test_reflection_arena.h"
#include <robin/reflection/namespace.h>
#include <robin/frontends/simple/simplefrontend.h>
#include <robin/frontends/framework.h>

#include <robin/debug/trace.h>


int main(int argc, char *argv[])
{
    Handle<Robin::Frontend> fe(new Robin::SimpleFrontend);

    Robin::FrontendsFramework::selectFrontend(fe);
    registerEnterprise();

    /* A '+' as a command line argument enables debug tracing */
    if (argc >= 2 && argv[1][0] == '+') {
		Robin::dbg::trace.enable();
		--argc; ++argv; /* shift */
    }

    /* The name of a test indicates: run this test only.
     * If none specified - run all tests. */
    bool allok;
    if (argc < 2)
    {
    	allok = Extreme::TestingProgram::engage();
    }
    else
    {
    	allok =Extreme::TestingProgram::engage(argv[1]);
    }

    if(allok) {
    	return 0;
    } else {
    	return 1;
    }
}

