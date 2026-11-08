%option noyywrap nounput noinput

%{
#include <string>
#include "ast.hpp"
#include "parser.tab.hpp"
%}

DIGIT   [0-9]
IDSTART [a-zA-Z_]
IDCHAR  [a-zA-Z_0-9]
SUFFIX  [uUlL]*

%%
"/*"([^*]|"*"+[^*/])*"*"+"/"   ;
"//".*                         ;
[ \t\r\n]+                     ;

"int"       { return T_INT; }
"unsigned"  { return T_UNSIGNED; }
"void"      { return T_VOID; }
"return"    { return T_RETURN; }
"if"        { return T_IF; }
"else"      { return T_ELSE; }
"while"     { return T_WHILE; }
"do"        { return T_DO; }
"for"       { return T_FOR; }
"break"     { return T_BREAK; }
"continue"  { return T_CONTINUE; }

0[xX][0-9a-fA-F]+{SUFFIX}  { yylval.str = new std::string(yytext); return INT_LITERAL; }
{DIGIT}+{SUFFIX}           { yylval.str = new std::string(yytext); return INT_LITERAL; }
{IDSTART}{IDCHAR}*         { yylval.str = new std::string(yytext); return IDENTIFIER; }

"<<="  { return LEFT_ASSIGN; }
">>="  { return RIGHT_ASSIGN; }
"+="   { return ADD_ASSIGN; }
"-="   { return SUB_ASSIGN; }
"*="   { return MUL_ASSIGN; }
"/="   { return DIV_ASSIGN; }
"%="   { return MOD_ASSIGN; }
"&="   { return AND_ASSIGN; }
"^="   { return XOR_ASSIGN; }
"|="   { return OR_ASSIGN; }
"<<"   { return LEFT_OP; }
">>"   { return RIGHT_OP; }
"<="   { return LE_OP; }
">="   { return GE_OP; }
"=="   { return EQ_OP; }
"!="   { return NE_OP; }
"&&"   { return AND_OP; }
"||"   { return OR_OP; }
"++"   { return INC_OP; }
"--"   { return DEC_OP; }

[-+*/%<>=!~&|^?:;,(){}]  { return yytext[0]; }

.      { fprintf(stderr, "lexer: unexpected character '%s'\n", yytext); exit(1); }
%%
