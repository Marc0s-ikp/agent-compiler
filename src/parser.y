%code requires {
#include <memory>
#include <string>
#include <vector>
#include "ast.hpp"
}

%code {
#include <cstdio>
#include "typedef_table.hpp"

int yylex();
void yyerror(std::unique_ptr<ast::TranslationUnit>& root, const char* msg);

// A declaration is complete: leave typedef mode, and forget the parameter
// names of any prototype inside it (only a function body claims them).
static void EndDeclaration() {
    Typedefs().SetInTypedef(false);
    Typedefs().ClearPending();
}

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
    ast::Enumerator* enumerator;
    ast::EnumSpec* enum_spec;
    std::vector<ast::Enumerator>* enumerators;
    ast::Initializer* init;
    ast::Initializer::List* inits;
    std::vector<ast::StmtPtr>* stmts;
    std::vector<ast::ExprPtr>* args;
    std::vector<ast::ExternalDeclPtr>* externals;
    std::vector<ast::Param>* params;
    std::vector<ast::InitDeclarator>* declarators;
    std::string* str;
    int stars;
    ast::Storage storage;
}

%token <str> IDENTIFIER INT_LITERAL T_TYPE_NAME
%token <str> CHAR_LITERAL STRING_LITERAL
%token T_INT T_UNSIGNED T_SIGNED T_CHAR T_SIZEOF T_VOID T_RETURN T_IF T_ELSE T_WHILE T_DO T_FOR
%token T_BREAK T_CONTINUE T_EXTERN T_STATIC T_ENUM T_TYPEDEF T_SWITCH T_CASE T_DEFAULT
%token LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP LEFT_OP RIGHT_OP INC_OP DEC_OP
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN

%type <expr> expr_opt expr assignment_expr conditional_expr binary_expr unary_expr
%type <expr> postfix_expr primary_expr
%type <stmt> statement declaration
%type <compound> compound_stmt
%type <external> external_decl
%type <param> param_decl type_name
%type <declarator> init_declarator
%type <stmts> statement_list
%type <args> argument_list argument_list_opt
%type <externals> external_list
%type <params> param_list param_list_opt
%type <declarators> init_declarator_list
%type <spec> type_spec
%type <enum_spec> enum_spec
%type <enumerator> enumerator
%type <enumerators> enumerator_list
%type <name_decl> declarator direct_declarator abstract_declarator direct_abstract
%type <stars> pointer
%type <storage> storage local_storage
%type <str> name string_literal
%type <name_decl> declarator_head
%type <init> initializer
%type <inits> initializer_list

%destructor { delete $$; } <expr> <stmt> <compound> <external> <param> <declarator> <spec> <name_decl> <init> <enum_spec> <enumerator>
%destructor { delete $$; } <stmts> <externals> <params> <declarators> <str> <args> <inits> <enumerators>

/* Dangling else: shift the 'else' so it binds to the nearest 'if'. */
%nonassoc LOWER_THAN_ELSE
%nonassoc T_ELSE

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
    : external_decl {
        $$ = new std::vector<ast::ExternalDeclPtr>();
        $$->push_back(ast::ExternalDeclPtr($1));
    }
    | external_list external_decl {
        $$ = $1;
        $$->push_back(ast::ExternalDeclPtr($2));
    }
    ;

external_decl
    : storage type_spec init_declarator_list ';' {
        EndDeclaration();
        $$ = new ast::GlobalDeclaration(*$2, std::move(*$3), $1);
        delete $2;
        delete $3;
    }
    | storage type_spec ';' {
        EndDeclaration();
        $$ = new ast::GlobalDeclaration(*$2, {}, $1);
        delete $2;
    }
    | storage type_spec declarator compound_stmt {
        $$ = new ast::FunctionDefinition(*$2, std::move(*$3),
                                         std::unique_ptr<ast::CompoundStmt>($4), $1);
        delete $2;
        delete $3;
    }
    ;

storage
    : /* empty */   { $$ = ast::Storage::None; }
    | T_EXTERN      { $$ = ast::Storage::Extern; }
    | local_storage
    ;

local_storage
    : T_STATIC      { $$ = ast::Storage::Static; }
    | T_TYPEDEF     { $$ = ast::Storage::Typedef; Typedefs().SetInTypedef(true); }
    ;

type_spec
    : T_INT              { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Int); }
    | T_UNSIGNED         { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Unsigned); }
    | T_UNSIGNED T_INT   { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Unsigned); }
    | T_SIGNED           { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Int); }
    | T_SIGNED T_INT     { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Int); }
    | T_CHAR             { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Char); }
    | T_SIGNED T_CHAR    { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::SignedChar); }
    | T_UNSIGNED T_CHAR  { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::UnsignedChar); }
    | T_VOID             { $$ = new ast::TypeSpecifier(ast::TypeSpecifier::Kind::Void); }
    | T_TYPE_NAME        { $$ = new ast::TypeSpecifier(std::move(*$1)); delete $1; }
    | enum_spec {
        $$ = new ast::TypeSpecifier(std::shared_ptr<const ast::EnumSpec>($1));
    }
    ;

/* A name being declared. It may spell a typedef name from an outer scope
   ('int T;' hides it), so the lexer's T_TYPE_NAME is accepted too. */
name
    : IDENTIFIER
    | T_TYPE_NAME
    ;

enum_spec
    : T_ENUM name {
        $$ = new ast::EnumSpec();
        $$->tag = std::move(*$2);
        delete $2;
    }
    | T_ENUM '{' enumerator_list opt_comma '}' {
        $$ = new ast::EnumSpec();
        $$->has_body = true;
        $$->items = std::move(*$3);
        delete $3;
    }
    | T_ENUM name '{' enumerator_list opt_comma '}' {
        $$ = new ast::EnumSpec();
        $$->tag = std::move(*$2);
        $$->has_body = true;
        $$->items = std::move(*$4);
        delete $2;
        delete $4;
    }
    ;

opt_comma
    : /* empty */
    | ','
    ;

enumerator_list
    : enumerator {
        $$ = new std::vector<ast::Enumerator>();
        $$->push_back(std::move(*$1));
        delete $1;
    }
    | enumerator_list ',' enumerator {
        $$ = $1;
        $$->push_back(std::move(*$3));
        delete $3;
    }
    ;

enumerator
    : name {
        Typedefs().Declare(*$1, false);
        $$ = new ast::Enumerator{std::move(*$1), nullptr};
        delete $1;
    }
    | name '=' conditional_expr {
        Typedefs().Declare(*$1, false);
        $$ = new ast::Enumerator{std::move(*$1), Own($3)};
        delete $1;
    }
    ;

declarator
    : direct_declarator
    | pointer direct_declarator {
        $$ = $2;
        for (int i = 0; i < $1; ++i) {
            $$ = new ast::Declarator(ast::Declarator::Pointer(
                std::unique_ptr<ast::Declarator>($$)));
        }
    }
    ;

pointer
    : '*'           { $$ = 1; }
    | pointer '*'   { $$ = $1 + 1; }
    ;

direct_declarator
    : name {
        $$ = new ast::Declarator(ast::Declarator::Name(std::move(*$1)));
        delete $1;
    }
    | '(' declarator ')' { $$ = $2; }
    | direct_declarator '[' conditional_expr ']' {
        $$ = new ast::Declarator(ast::Declarator::Array(
            std::unique_ptr<ast::Declarator>($1), Own($3)));
    }
    | direct_declarator '[' ']' {
        $$ = new ast::Declarator(ast::Declarator::Array(
            std::unique_ptr<ast::Declarator>($1), nullptr));
    }
    | direct_declarator '(' param_list_opt ')' {
        $$ = new ast::Declarator(ast::Declarator::Function(
            std::unique_ptr<ast::Declarator>($1), std::move(*$3)));
        delete $3;
    }
    ;

/* A type name, as in sizeof(int *[5]): a specifier and an abstract
   declarator (a declarator without the identifier). Function declarators
   are not supported here. */
type_name
    : type_spec {
        $$ = new ast::Param{*$1, ast::Declarator::Name("")};
        delete $1;
    }
    | type_spec abstract_declarator {
        $$ = new ast::Param{*$1, std::move(*$2)};
        delete $1;
        delete $2;
    }
    ;

abstract_declarator
    : pointer {
        $$ = new ast::Declarator(ast::Declarator::Name(""));
        for (int i = 0; i < $1; ++i) {
            $$ = new ast::Declarator(ast::Declarator::Pointer(
                std::unique_ptr<ast::Declarator>($$)));
        }
    }
    | direct_abstract
    | pointer direct_abstract {
        $$ = $2;
        for (int i = 0; i < $1; ++i) {
            $$ = new ast::Declarator(ast::Declarator::Pointer(
                std::unique_ptr<ast::Declarator>($$)));
        }
    }
    ;

direct_abstract
    : '(' abstract_declarator ')' { $$ = $2; }
    | '[' conditional_expr ']' {
        $$ = new ast::Declarator(ast::Declarator::Array(
            std::make_unique<ast::Declarator>(ast::Declarator::Name("")), Own($2)));
    }
    | direct_abstract '[' conditional_expr ']' {
        $$ = new ast::Declarator(ast::Declarator::Array(
            std::unique_ptr<ast::Declarator>($1), Own($3)));
    }
    ;

param_list_opt
    : /* empty */   { $$ = new std::vector<ast::Param>(); }
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
    : type_spec {
        $$ = new ast::Param{*$1, ast::Declarator::Name("")};
        delete $1;
    }
    | type_spec pointer {
        std::unique_ptr<ast::Declarator> d(
            new ast::Declarator(ast::Declarator::Name("")));
        for (int i = 0; i < $2; ++i) {
            d.reset(new ast::Declarator(ast::Declarator::Pointer(std::move(d))));
        }
        $$ = new ast::Param{*$1, std::move(*d)};
        delete $1;
    }
    | type_spec declarator {
        if (!$2->DeclaredName().empty()) {
            Typedefs().AddPendingParam($2->DeclaredName());
        }
        $$ = new ast::Param{*$1, std::move(*$2)};
        delete $1;
        delete $2;
    }
    ;

compound_stmt
    : '{' { Typedefs().PushScope(); } statement_list '}' {
        Typedefs().PopScope();
        $$ = new ast::CompoundStmt(std::move(*$3));
        delete $3;
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
    | T_IF '(' expr ')' statement %prec LOWER_THAN_ELSE {
        $$ = new ast::IfStmt(Own($3), Own($5), nullptr);
    }
    | T_IF '(' expr ')' statement T_ELSE statement {
        $$ = new ast::IfStmt(Own($3), Own($5), Own($7));
    }
    | T_WHILE '(' expr ')' statement {
        $$ = new ast::WhileStmt(Own($3), Own($5));
    }
    | T_DO statement T_WHILE '(' expr ')' ';' {
        $$ = new ast::DoWhileStmt(Own($2), Own($5));
    }
    | T_FOR '(' expr_opt ';' expr_opt ';' expr_opt ')' statement {
        $$ = new ast::ForStmt(Own($3), Own($5), Own($7), Own($9));
    }
    | T_SWITCH '(' expr ')' statement {
        $$ = new ast::SwitchStmt(Own($3), Own($5));
    }
    | T_CASE conditional_expr ':' statement {
        $$ = new ast::CaseStmt(Own($2), Own($4));
    }
    | T_DEFAULT ':' statement {
        $$ = new ast::DefaultStmt(Own($3));
    }
    | T_BREAK ';'              { $$ = new ast::BreakStmt(); }
    | T_CONTINUE ';'           { $$ = new ast::ContinueStmt(); }
    | T_RETURN ';'             { $$ = new ast::ReturnStmt(nullptr); }
    | T_RETURN expr ';'        { $$ = new ast::ReturnStmt(Own($2)); }
    ;

declaration
    : type_spec init_declarator_list ';' {
        EndDeclaration();
        $$ = new ast::DeclarationStmt(*$1, std::move(*$2), ast::Storage::None);
        delete $1;
        delete $2;
    }
    | type_spec ';' {
        EndDeclaration();
        $$ = new ast::DeclarationStmt(*$1, {}, ast::Storage::None);
        delete $1;
    }
    | local_storage type_spec init_declarator_list ';' {
        EndDeclaration();
        $$ = new ast::DeclarationStmt(*$2, std::move(*$3), $1);
        delete $2;
        delete $3;
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

/* The name is declared (as a type or as an ordinary identifier) as soon as
   its declarator is complete, before the initialiser and before the next
   declaration is lexed. */
declarator_head
    : declarator {
        Typedefs().Declare($1->DeclaredName(), Typedefs().InTypedef());
        $$ = $1;
    }
    ;

init_declarator
    : declarator_head {
        $$ = new ast::InitDeclarator{std::move(*$1), nullptr};
        delete $1;
    }
    | declarator_head '=' initializer {
        $$ = new ast::InitDeclarator{
            std::move(*$1), std::unique_ptr<ast::Initializer>($3)};
        delete $1;
    }
    ;

initializer
    : assignment_expr { $$ = new ast::Initializer(Own($1)); }
    | '{' initializer_list '}'     { $$ = new ast::Initializer(std::move(*$2)); delete $2; }
    | '{' initializer_list ',' '}' { $$ = new ast::Initializer(std::move(*$2)); delete $2; }
    ;

initializer_list
    : initializer {
        $$ = new ast::Initializer::List();
        $$->push_back(std::unique_ptr<ast::Initializer>($1));
    }
    | initializer_list ',' initializer {
        $$ = $1;
        $$->push_back(std::unique_ptr<ast::Initializer>($3));
    }
    ;

expr_opt
    : /* empty */ { $$ = nullptr; }
    | expr
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
    | '*' unary_expr     { $$ = new ast::Deref(Own($2)); }
    | '&' unary_expr     { $$ = new ast::AddressOf(Own($2)); }
    | T_SIZEOF unary_expr { $$ = new ast::SizeofExpr(Own($2)); }
    | T_SIZEOF '(' type_name ')' {
        $$ = new ast::SizeofType(std::move(*$3));
        delete $3;
    }
    ;

postfix_expr
    : primary_expr
    | IDENTIFIER '(' argument_list_opt ')' {
        $$ = new ast::FunctionCall(std::move(*$1), std::move(*$3));
        delete $1;
        delete $3;
    }
    | postfix_expr '[' expr ']' { $$ = new ast::Index(Own($1), Own($3)); }
    | postfix_expr INC_OP { $$ = new ast::IncDec(true, false, Own($1)); }
    | postfix_expr DEC_OP { $$ = new ast::IncDec(false, false, Own($1)); }
    ;

argument_list_opt
    : /* empty */   { $$ = new std::vector<ast::ExprPtr>(); }
    | argument_list { $$ = $1; }
    ;

argument_list
    : assignment_expr {
        $$ = new std::vector<ast::ExprPtr>();
        $$->push_back(Own($1));
    }
    | argument_list ',' assignment_expr {
        $$ = $1;
        $$->push_back(Own($3));
    }
    ;

/* Adjacent string literals are one literal (translation phase 6). */
string_literal
    : STRING_LITERAL
    | string_literal STRING_LITERAL {
        $$ = $1;
        $$->append(*$2);
        delete $2;
    }
    ;

primary_expr
    : IDENTIFIER   { $$ = new ast::Identifier(std::move(*$1)); delete $1; }
    | INT_LITERAL  { $$ = new ast::IntLiteral(*$1); delete $1; }
    | string_literal {
        $$ = new ast::StringLiteral(std::move(*$1));
        delete $1;
    }
    | CHAR_LITERAL {
        $$ = new ast::CharLiteral(static_cast<unsigned char>((*$1)[0]));
        delete $1;
    }
    | '(' expr ')' { $$ = $2; }
    ;

%%

void yyerror(std::unique_ptr<ast::TranslationUnit>&, const char* msg) {
    std::fprintf(stderr, "parse error: %s\n", msg);
}
