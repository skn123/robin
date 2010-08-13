/* -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

#ifndef ROBIN_EXTREME_INTERACTIVE_SIMPLE_SYNTAX_H
#define ROBIN_EXTREME_INTERACTIVE_SIMPLE_SYNTAX_H

#ifndef yyFlexLexer
#define yyFlexLexer SimpleInteractiveFlexLexer
#include <FlexLexer.h>
#endif

/* This defined is needed in flex2.5.33 and it provides
 * access to what in flex2.5.3 was yy_current_buffer
 * (notice yy_buffer_stack is a member of yyFlexLexer
 * so the macro is valid from methods of classes
 * inheriting from it).
 */
#define YY_CURRENT_BUFFER ( (yy_buffer_stack) \
                          ? (yy_buffer_stack)[(yy_buffer_stack_top)] \
                          : NULL)



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
	void reuse_address(int cell);
	void reuse_by_address(int cell);
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
	void address(const char *name);
	void dereference(const char *name);
	void assign(const char *name);

	int var_lookup(const char *name);

	void IncludeIn(const char *filename, int times);
	bool IncludeOut();

private:
	struct Imp;
	InteractiveSyntaxAnalyzer::Imp *imp;

	// Inclusion support in lexer
	IncludeStack include;
};


#endif
