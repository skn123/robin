// -*- C++ -*-

/**
 * @par SOURCE
 * fwtesting.h
 *
 * @par TITLE
 * Testing Framework
 */

#ifndef EXTREME_TESTING_FW_H
#define EXTREME_TESTING_FW_H

#include <string>
#include <sstream>
#include <vector>
#include <map>

namespace Extreme {

/**
 * @class AbstractTest
 * @nosubgrouping
 *
 * Generic test interface.
 */
class AbstractTest
{
public:
    virtual void prepare() = 0;
    virtual bool errorExpected() = 0;
    virtual void go() = 0;
    virtual void alternate() = 0;
    virtual void unexpectedError() = 0;
    virtual bool verify() = 0;

    virtual const char *title() const = 0;
    virtual void report() const = 0;
    virtual ~AbstractTest() =0 ;
};

inline AbstractTest::~AbstractTest()
{

}

/**
 * @class Test
 * @nosubgrouping
 *
 * A partial implementation of the AbstractTest
 */
class Test : public AbstractTest
{
public:
    virtual void prepare();
    virtual void alternate();
    virtual bool errorExpected();
    virtual void unexpectedError();
    virtual void report() const;

    template < class T >
    void notification(std::string name, T value);

protected:
    Test();
    Test(const char *title);

    const char *m_title;
    std::vector<std::string> m_report;
    std::map<std::string, int> m_reported;

public:
    virtual const char *title() const { return m_title; }
};

/**
 * @class TestingProgram
 * @nosubgrouping
 *
 * Holds global and common test attributes.
 */
class TestingProgram
{
private:
    static std::vector<AbstractTest *> registered_tests;

public:
    static void registerUnit(AbstractTest *test);
    /**
     * Runs the test, returns true only if everything
     * went OK
     */
    static bool engage(const char *criteria = NULL);
};




template < class T >
void Test::notification(std::string name, T value)
{
    std::stringstream ss;
    ss << "// @" << name << ": " << value;

    if (m_reported[name] == 0) {
	m_reported[name] = m_report.size();
	m_report.push_back(ss.str());
    }
    else {
	m_report[m_reported[name]] = ss.str();
    }
}

} // end of namespace Extreme

#endif
