/* -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

#ifndef ROBIN_EXTREME_INTERACTIVE_SIMPLE_SYNTAX_H
#define ROBIN_EXTREME_INTERACTIVE_SIMPLE_SYNTAX_H

#ifndef yyFlexLexer
#define yyFlexLexer SimpleInteractiveFlexLexer
#include <FlexLexer.h>
#endif


#include <pattern/handle.h>
#include <string>

#include "inclusion.h"


namespace Robin { class Namespace; }
namespace Simple { class Element; }

/**
 * @class InteractiveSyntaxAnalyzer
 * @nosubgrouping
 *
 * Controls tokens received from the interactive input,
 * processes them and issues the approperiate invocations.
 */
class InteractiveSyntaxAnalyzer : public SimpleInteractiveFlexLexer
{
public:
	InteractiveSyntaxAnalyzer();

	int yylex();

	void have(Handle<Robin::Namespace> globals);

	void expose();
	void import();

	void identifier(const char *name);
	void literal(int value);
	void literal(float value);
	void literal(std::string value);

	void reuse();
	void reuse(int cell);
	void var(int cell);

	void enter();
	void exit();

	void period();

	void actNothing();
	void actFunctionCall();
	void actImportModule();

	Simple::Element *result() const;

protected:
	void string(const char *phrase);

	void blank();
	void variable(const char *name);
	void assign(const char *name);

	void IncludeIn(const char *filename, int times);
	bool IncludeOut();

private:
	struct Imp;
	InteractiveSyntaxAnalyzer::Imp *imp;

	// Inclusion support in lexer
	IncludeStack include;
};


#endif
