/* -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

#include <stdio.h>
#include <fstream>

// - needed for creating the sample namespace
#include <robin/reflection/callable.h>
#include <robin/reflection/namespace.h>
#include <robin/reflection/cfunction.h>
#include <robin/reflection/intrinsic_type_arguments.h>

#include <robin/frontends/framework.h>
#include <robin/frontends/simple/simplefrontend.h>

#include <robin/debug/trace.h>

#include "syntax.h"
#include "../test_reflection_arena.h"


int my_do()
{
	printf("Howdie!\n");
	return 42;
}

/**
 * @par TEMPORARY
 * just for the testing
 */
Handle<Robin::Namespace> sample_ns()
{
	Handle<Robin::CFunction> 
		hdo(new Robin::CFunction((Robin::symbol)&my_do));
	hdo->specifyReturnType(Robin::ArgumentInt);

	Handle<Robin::Namespace> ns(new Robin::Namespace);
	ns->declare("do", static_hcast<Robin::Callable>(hdo));

	return ns;
}


/**
 */
int main(int argc, char *argv[])
{
	if (argc > 1 && argv[1][0] == '+') {
		dbg::trace.enable();
		--argc; ++argv; /* shift */
	}

    Handle<Robin::Frontend> fe(new Robin::SimpleFrontend);

    Robin::FrontendsFramework::selectFrontend(fe);

	InteractiveSyntaxAnalyzer interpreter;

	// Prepare a sample namespace
	// InteractiveSyntaxAnalyzer::getActive().have(registerEnterprise());

	// Read the input file(s)
	while (argc > 1) {
		std::ifstream input(argv[1]);
		if (input.good()) {
			interpreter.yyrestart(&input);
			interpreter.yylex();
			interpreter.yyrestart(&std::cin);
		}
		else {
			perror(argv[1]);
		}
		--argc; ++argv; /* shift */
	}

	interpreter.yylex();
	return 0;
}

