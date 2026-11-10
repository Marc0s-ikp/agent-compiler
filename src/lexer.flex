%option noyywrap nounput noinput

%{
#include <string>
#include "ast.hpp"
#include "parser.tab.hpp"
#include "typedef_table.hpp"
#include "escapes.hpp"
#include <cstdlib>
#include <stdexcept>

// The decoded text between the quotes of yytext. A bad escape ends the
// compile here, like any other lexical error.
static std::string* Decoded() {
    try {
        return new std::string(DecodeEscapes(std::string(yytext + 1, yyleng - 2)));
    } catch (const std::exception& e) {
        fprintf(stderr, "lexer: %s\n", e.what());
        exit(1);
    }
}
%}

DIGIT   [0-9]
IDSTART [a-zA-Z_]
IDCHAR  [a-zA-Z_0-9]
SUFFIX  [uUlL]*
EXPONENT [eE][+-]?[0-9]+
FSUFFIX [fFlL]

%%
"/*"([^*]|"*"+[^*/])*"*"+"/"   ;
"//".*                         ;
[ \t\r\n]+                     ;

"int"       { return T_INT; }
"unsigned"  { return T_UNSIGNED; }
"signed"    { return T_SIGNED; }
"char"      { return T_CHAR; }
"float"     { return T_FLOAT; }
"double"    { return T_DOUBLE; }
"sizeof"    { return T_SIZEOF; }
"void"      { return T_VOID; }
"return"    { return T_RETURN; }
"if"        { return T_IF; }
"else"      { return T_ELSE; }
"while"     { return T_WHILE; }
"do"        { return T_DO; }
"for"       { return T_FOR; }
"break"     { return T_BREAK; }
"continue"  { return T_CONTINUE; }
"extern"    { return T_EXTERN; }
"static"    { return T_STATIC; }
"enum"      { return T_ENUM; }
"struct"    { return T_STRUCT; }
"union"     { return T_UNION; }
"typedef"   { return T_TYPEDEF; }
"switch"    { return T_SWITCH; }
"case"      { return T_CASE; }
"default"   { return T_DEFAULT; }

{DIGIT}+{EXPONENT}{FSUFFIX}?              { yylval.str = new std::string(yytext); return FLOAT_LITERAL; }
{DIGIT}*"."{DIGIT}+{EXPONENT}?{FSUFFIX}?   { yylval.str = new std::string(yytext); return FLOAT_LITERAL; }
{DIGIT}+"."{DIGIT}*{EXPONENT}?{FSUFFIX}?   { yylval.str = new std::string(yytext); return FLOAT_LITERAL; }
0[xX][0-9a-fA-F]+{SUFFIX}  { yylval.str = new std::string(yytext); return INT_LITERAL; }
{DIGIT}+{SUFFIX}           { yylval.str = new std::string(yytext); return INT_LITERAL; }
\'([^'\\\n]|\\.)*\'  {
    yylval.str = Decoded();
    if (yylval.str->size() != 1) {
        fprintf(stderr, "lexer: bad character constant %s\n", yytext);
        exit(1);
    }
    return CHAR_LITERAL;
}
\"([^"\\\n]|\\.)*\"  { yylval.str = Decoded(); return STRING_LITERAL; }
{IDSTART}{IDCHAR}*         {
    yylval.str = new std::string(yytext);
    return Typedefs().IsTypeName(yytext) ? T_TYPE_NAME : IDENTIFIER;
}

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
"->"   { return PTR_OP; }
"."    { return '.'; }

[-+*/%<>=!~&|^?:;,(){}\[\]]  { return yytext[0]; }

.      { fprintf(stderr, "lexer: unexpected character '%s'\n", yytext); exit(1); }
%%
