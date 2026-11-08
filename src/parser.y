%code requires {
#include <memory>
#include <string>
#include <vector>
#include "ast.hpp"
}

%code {
#include <cstdio>

int yylex();
void yyerror(std::unique_ptr<ast::TranslationUnit>& root, const char* msg);

static ast::ExprPtr Own(ast::Expr* e) { return ast::ExprPtr(e); }
static ast::StmtPtr Own(ast::Stmt* s) { return ast::StmtPtr(s); }
}

%parse-param { std::unique_ptr<ast::TranslationUnit>& root }

%union {
    ast::Expr* expr;
    ast::Stmt* stmt;
    ast::CompoundStmt* compound;
    ast::ExternalDecl* external;
    ast::Param* param;
    ast::TypeSpecifier* spec;
    ast::InitDeclarator* declarator;
    ast::Declarator* name_decl;
    std::vector<ast::StmtPtr>* stmts;
    std::vector<ast::ExternalDeclPtr>* externals;
    std::vector<ast::Param>* params;
    std::vector<ast::InitDeclarator>* declarators;
    std::string* str;
}

%token <str> IDENTIFIER INT_LITERAL
%token T_INT T_UNSIGNED T_VOID T_RETURN
%token LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP LEFT_OP RIGHT_OP INC_OP DEC_OP
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN

%type <expr> expr assignment_expr conditional_expr binary_expr unary_expr
%type <expr> postfix_expr primary_expr
%type <stmt> statement declaration
%type <compound> compound_stmt
%type <external> function_definition
%type <param> param_decl
%type <declarator> init_declarator
%type <stmts> statement_list
%type <externals> external_list
%type <params> param_list param_list_opt
%type <declarators> init_declarator_list
%type <spec> type_spec
%type <name_decl> declarator

%destructor { delete $$; } <expr> <stmt> <compound> <external> <param> <declarator> <spec> <name_decl>
%destructor { delete $$; } <stmts> <externals> <params> <declarators> <str>

/* Binary operator precedence, lowest first. */
%left OR_OP
%left AND_OP
%left '|'
%left '^'
%left '&'
%left EQ_OP NE_OP
%left '<' '>' LE_OP GE_OP
%left LEFT_OP RIGHT_OP
%left '+' '-'
%left '*' '/' '%'

%start translation_unit

%%

translation_unit
    : external_list {
        root = std::make_unique<ast::TranslationUnit>(std::move(*$1));
        delete $1;
    }
    ;

external_list
    : function_definition {
        $$ = new std::vector<ast::ExternalDeclPtr>();
        $$->push_back(ast::ExternalDeclPtr($1));
    }
    | external_list function_definition {
        $$ = $1;
        $$->push_back(ast::ExternalDeclPtr($2));
    }
    ;

function_definition
    : type_spec declarator '(' param_list_opt ')' compound_stmt {
        $$ = new ast::FunctionDefinition(*$1, std::move(*$2), std::move(*$4),
                                         std::unique_ptr<ast::CompoundStmt>($6));
        delete $1;
        delete $2;
        delete $4;
    }
    ;

type_spec
    : T_INT              { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Int); }
    | T_UNSIGNED         { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Unsigned); }
    | T_UNSIGNED T_INT   { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Unsigned); }
    | T_VOID             { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Void); }
    ;

declarator
    : IDENTIFIER { $$ = new ast::Declarator(std::move(*$1)); delete $1; }
    ;

param_list_opt
    : /* empty */   { $$ = new std::vector<ast::Param>(); }
    | T_VOID        { $$ = new std::vector<ast::Param>(); }
    | param_list    { $$ = $1; }
    ;

param_list
    : param_decl {
        $$ = new std::vector<ast::Param>();
        $$->push_back(std::move(*$1));
        delete $1;
    }
    | param_list ',' param_decl {
        $$ = $1;
        $$->push_back(std::move(*$3));
        delete $3;
    }
    ;

param_decl
    : type_spec declarator {
        $$ = new ast::Param{*$1, std::move(*$2)};
        delete $1;
        delete $2;
    }
    ;

compound_stmt
    : '{' statement_list '}' {
        $$ = new ast::CompoundStmt(std::move(*$2));
        delete $2;
    }
    ;

statement_list
    : /* empty */ { $$ = new std::vector<ast::StmtPtr>(); }
    | statement_list statement {
        $$ = $1;
        $$->push_back(Own($2));
    }
    ;

statement
    : compound_stmt            { $$ = $1; }
    | declaration              { $$ = $1; }
    | expr ';'                 { $$ = new ast::ExprStmt(Own($1)); }
    | ';'                      { $$ = new ast::ExprStmt(nullptr); }
    | T_RETURN ';'             { $$ = new ast::ReturnStmt(nullptr); }
    | T_RETURN expr ';'        { $$ = new ast::ReturnStmt(Own($2)); }
    ;

declaration
    : type_spec init_declarator_list ';' {
        $$ = new ast::DeclarationStmt(*$1, std::move(*$2));
        delete $1;
        delete $2;
    }
    ;

init_declarator_list
    : init_declarator {
        $$ = new std::vector<ast::InitDeclarator>();
        $$->push_back(std::move(*$1));
        delete $1;
    }
    | init_declarator_list ',' init_declarator {
        $$ = $1;
        $$->push_back(std::move(*$3));
        delete $3;
    }
    ;

init_declarator
    : declarator {
        $$ = new ast::InitDeclarator{std::move(*$1), nullptr};
        delete $1;
    }
    | declarator '=' assignment_expr {
        $$ = new ast::InitDeclarator{
            std::move(*$1), std::make_unique<ast::Initializer>(Own($3))};
        delete $1;
    }
    ;

expr
    : assignment_expr
    | expr ',' assignment_expr { $$ = new ast::Comma(Own($1), Own($3)); }
    ;

assignment_expr
    : conditional_expr
    | unary_expr '=' assignment_expr {
        $$ = new ast::Assign(Own($1), Own($3));
    }
    | unary_expr ADD_ASSIGN assignment_expr   { $$ = new ast::CompoundAssign(ast::BinOp::Add, Own($1), Own($3)); }
    | unary_expr SUB_ASSIGN assignment_expr   { $$ = new ast::CompoundAssign(ast::BinOp::Sub, Own($1), Own($3)); }
    | unary_expr MUL_ASSIGN assignment_expr   { $$ = new ast::CompoundAssign(ast::BinOp::Mul, Own($1), Own($3)); }
    | unary_expr DIV_ASSIGN assignment_expr   { $$ = new ast::CompoundAssign(ast::BinOp::Div, Own($1), Own($3)); }
    | unary_expr MOD_ASSIGN assignment_expr   { $$ = new ast::CompoundAssign(ast::BinOp::Mod, Own($1), Own($3)); }
    | unary_expr LEFT_ASSIGN assignment_expr  { $$ = new ast::CompoundAssign(ast::BinOp::Shl, Own($1), Own($3)); }
    | unary_expr RIGHT_ASSIGN assignment_expr { $$ = new ast::CompoundAssign(ast::BinOp::Shr, Own($1), Own($3)); }
    | unary_expr AND_ASSIGN assignment_expr   { $$ = new ast::CompoundAssign(ast::BinOp::And, Own($1), Own($3)); }
    | unary_expr XOR_ASSIGN assignment_expr   { $$ = new ast::CompoundAssign(ast::BinOp::Xor, Own($1), Own($3)); }
    | unary_expr OR_ASSIGN assignment_expr    { $$ = new ast::CompoundAssign(ast::BinOp::Or, Own($1), Own($3)); }
    ;

conditional_expr
    : binary_expr
    | binary_expr '?' expr ':' conditional_expr {
        $$ = new ast::Ternary(Own($1), Own($3), Own($5));
    }
    ;

binary_expr
    : unary_expr
    | binary_expr OR_OP binary_expr   { $$ = new ast::LogicalOr(Own($1), Own($3)); }
    | binary_expr AND_OP binary_expr  { $$ = new ast::LogicalAnd(Own($1), Own($3)); }
    | binary_expr '|' binary_expr     { $$ = new ast::BinaryArith(ast::BinOp::Or, Own($1), Own($3)); }
    | binary_expr '^' binary_expr     { $$ = new ast::BinaryArith(ast::BinOp::Xor, Own($1), Own($3)); }
    | binary_expr '&' binary_expr     { $$ = new ast::BinaryArith(ast::BinOp::And, Own($1), Own($3)); }
    | binary_expr EQ_OP binary_expr   { $$ = new ast::BinaryCompare(ast::CmpOp::Eq, Own($1), Own($3)); }
    | binary_expr NE_OP binary_expr   { $$ = new ast::BinaryCompare(ast::CmpOp::Ne, Own($1), Own($3)); }
    | binary_expr '<' binary_expr     { $$ = new ast::BinaryCompare(ast::CmpOp::Lt, Own($1), Own($3)); }
    | binary_expr '>' binary_expr     { $$ = new ast::BinaryCompare(ast::CmpOp::Gt, Own($1), Own($3)); }
    | binary_expr LE_OP binary_expr   { $$ = new ast::BinaryCompare(ast::CmpOp::Le, Own($1), Own($3)); }
    | binary_expr GE_OP binary_expr   { $$ = new ast::BinaryCompare(ast::CmpOp::Ge, Own($1), Own($3)); }
    | binary_expr LEFT_OP binary_expr  { $$ = new ast::BinaryArith(ast::BinOp::Shl, Own($1), Own($3)); }
    | binary_expr RIGHT_OP binary_expr { $$ = new ast::BinaryArith(ast::BinOp::Shr, Own($1), Own($3)); }
    | binary_expr '+' binary_expr     { $$ = new ast::BinaryArith(ast::BinOp::Add, Own($1), Own($3)); }
    | binary_expr '-' binary_expr     { $$ = new ast::BinaryArith(ast::BinOp::Sub, Own($1), Own($3)); }
    | binary_expr '*' binary_expr     { $$ = new ast::BinaryArith(ast::BinOp::Mul, Own($1), Own($3)); }
    | binary_expr '/' binary_expr     { $$ = new ast::BinaryArith(ast::BinOp::Div, Own($1), Own($3)); }
    | binary_expr '%' binary_expr     { $$ = new ast::BinaryArith(ast::BinOp::Mod, Own($1), Own($3)); }
    ;

unary_expr
    : postfix_expr
    | INC_OP unary_expr  { $$ = new ast::IncDec(true, true, Own($2)); }
    | DEC_OP unary_expr  { $$ = new ast::IncDec(false, true, Own($2)); }
    | '+' unary_expr     { $$ = new ast::UnaryOp(ast::UnOp::Plus, Own($2)); }
    | '-' unary_expr     { $$ = new ast::UnaryOp(ast::UnOp::Neg, Own($2)); }
    | '~' unary_expr     { $$ = new ast::UnaryOp(ast::UnOp::BitNot, Own($2)); }
    | '!' unary_expr     { $$ = new ast::UnaryOp(ast::UnOp::LogNot, Own($2)); }
    ;

postfix_expr
    : primary_expr
    | postfix_expr INC_OP { $$ = new ast::IncDec(true, false, Own($1)); }
    | postfix_expr DEC_OP { $$ = new ast::IncDec(false, false, Own($1)); }
    ;

primary_expr
    : IDENTIFIER   { $$ = new ast::Identifier(std::move(*$1)); delete $1; }
    | INT_LITERAL  { $$ = new ast::IntLiteral(*$1); delete $1; }
    | '(' expr ')' { $$ = $2; }
    ;

%%

void yyerror(std::unique_ptr<ast::TranslationUnit>&, const char* msg) {
    std::fprintf(stderr, "parse error: %s\n", msg);
}
