// -*- C++ -*-

/**
 * @par SOURCE
 * fwtesting.cc
 */

#include "fwtesting.h"

#include <vector>
#include <iostream>

namespace Extreme {

  std::vector<AbstractTest *> TestingProgram::registered_tests;


void Test::prepare()
{
}

void Test::alternate()
{
}

bool Test::errorExpected()
{
    return false;
}

void Test::unexpectedError()
{
}

Test::Test()
    : m_title("(anonymous)")
{
    // Register
    TestingProgram::registerUnit(this);
}

Test::Test(const char *title)
    : m_title(title)
{
    // Register
    TestingProgram::registerUnit(this);
}

/**
 */
void Test::report() const
{
    for (std::vector<std::string>::const_iterator repline = m_report.begin();
	 repline != m_report.end(); ++repline) {
	std::cout << *repline << std::endl;
    }
}



/**
 * Registers a test object for the current testing
 * scheme.
 */
void TestingProgram::registerUnit(AbstractTest *test)
{
    registered_tests.push_back(test);
}

/**
 * Runs all the registered tests.
 */
void TestingProgram::engage(const char *criteria)
{
    static const char *sep = "//============================================"
	"==============================";

    int count_ok = 0;
    int count_failed = 0;

    for (std::vector<AbstractTest *>::iterator test_iter = 
	     registered_tests.begin();
	 test_iter != registered_tests.end(); ++test_iter)
    {
	if (criteria == NULL || 
	    strstr((*test_iter)->title(), criteria) != NULL)
	{
	    bool error_occurred = false;
	    const char *error_string = NULL;

	    // Print a nice title
	    std::cerr << sep << std::endl
		      << "// @TEST: " << (*test_iter)->title() << std::endl;
	    
	    // Perform the test
	    (*test_iter)->prepare();
	    try {
		(*test_iter)->go();
	    }
	    catch (std::exception& e) {
		error_occurred = true; error_string = e.what();
	    }
	    catch (...) {
		error_occurred = true; error_string = NULL;
	    }

	    (*test_iter)->alternate();

	    // Summarize outcome
	    if (error_occurred && !(*test_iter)->errorExpected()) {
		(*test_iter)->unexpectedError();
		std::cerr << "// @FAILED: "
			  << "Error occurred when it's not expected."
			  << std::endl;
		if (error_string != NULL)
		    std::cerr << "// @WHAT: " << error_string << std::endl;
		++count_failed;
	    }
	    else if (!(*test_iter)->verify()) {
		(*test_iter)->report();
		std::cerr << "// @FAILED: Verification failed." << std::endl;
		++count_failed;
	    } else {
		std::cerr << "// @OK: Successful." << std::endl;
		++count_ok;
	    }

	    std::cerr << sep << std::endl << std::endl;
	}
    }

    // Summarize statistics
    std::cerr << "" << std::endl << sep << std::endl
	      << "// @TOTAL:  " << (count_ok + count_failed) << std::endl
	      << "// @OK:     " << count_ok << std::endl
	      << "// @FAILED: " << count_failed << std::endl
	      << sep << std::endl;
}

} // end of namespace Extreme

