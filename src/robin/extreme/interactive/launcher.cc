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
#include "../../debug/trace.h"

int my_do()
{
	printf("Howdie!\n");
	return 42;
}

void my_other_do(const char **c)
{
	*c = "Howdie!";
}

/**
 * @par TEMPORARY
 * just for the testing
 */
Handle<Robin::Namespace> sample_ns()
{
	Handle<Robin::CFunction> 
		hdo(new Robin::CFunction((Robin::symbol)&my_do,"my_do",Robin::CFunction::GlobalFunction));
	Handle<Robin::OverloadedSet> hdo_overloaded(Robin::OverloadedSet::create_new("do"));
	hdo_overloaded->addAlternative(hdo);

	hdo->specifyReturnType(Robin::ArgumentInt);

	Handle<Robin::CFunction>
		hotdo(new Robin::CFunction((Robin::symbol)&my_other_do,"my_other_do",Robin::CFunction::GlobalFunction));
	hotdo->addFormalArgument(Robin::ArgumentCString->pointer());
	Handle<Robin::OverloadedSet> hotdo_overloaded(Robin::OverloadedSet::create_new("other_do"));
	hotdo_overloaded->addAlternative(hotdo);


	Handle<Robin::Namespace> ns(new Robin::Namespace("<sample>"));
	ns->declare("do", static_hcast<Robin::Callable>(hdo_overloaded));
	ns->declare("other_do", static_hcast<Robin::Callable>(hotdo_overloaded));

	Robin::FrontendsFramework::fillAdapter(Robin::ArgumentCString->pointer());

	return ns;
}


/**
 */
int main(int argc, char *argv[])
{
	if (argc > 1 && argv[1][0] == '+') {
		Robin::dbg::trace.enable();
		--argc; ++argv; /* shift */
	}

    Handle<Robin::Frontend> fe(new Robin::SimpleFrontend);

    Robin::FrontendsFramework::selectFrontend(fe);

	InteractiveSyntaxAnalyzer interpreter;

	// Prepare a sample namespace
	interpreter.have(sample_ns());

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

