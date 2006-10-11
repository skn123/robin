%option noyywrap
%option prefix="SimpleInteractive"
%option yyclass="InteractiveSyntaxAnalyzer"
%option c++

%{
#include "syntax.h"
#include "inclusion.h"
using std::cin;
using std::cout;
using std::cerr;
%}

VARIABLE [A-F]

%%

quit		{ return (0); }
import		{ import(); }

\<[a-z/]+	{ IncludeIn(yytext+1,1); }
[0-9]+\'[a-z/]+	{ IncludeIn(strchr(yytext,'\'')+1, atoi(yytext)); }

[A-F]		{ variable(yytext); }
&[A-F]		{ address(yytext+1); }
\*[A-F]		{ dereference(yytext+1); }
[A-F][:]	{ assign(yytext); }

_			{ blank(); }
[A-Za-z:_]+	{ identifier(yytext); }
[0-9]+		{ literal(atoi(yytext)); }
[0-9]+\.[0-9]+	{ literal((float)atof(yytext)); }
\"[^"]*\"	{ string(yytext); }

\(		{ enter(); }
\)		{ exit(); }

\.		{ period(); }

[ \t\r\n]	{ /* ignore */ }

<<EOF>>		{ if (!IncludeOut()) yyterminate(); }
