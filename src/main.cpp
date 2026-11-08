#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

#include "ast.hpp"


extern FILE* yyin;
int yylex_destroy();
int yyparse(std::unique_ptr<ast::TranslationUnit>& root);

namespace {

struct Options {
    const char* input = nullptr;
    const char* output = nullptr;
};

bool ParseArgs(int argc, char** argv, Options& opts) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-S") == 0) continue;
        if (std::strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            opts.output = argv[++i];
        } else {
            opts.input = argv[i];
        }
    }
    return opts.input && opts.output;
}

// Parse the input; returns null on a syntax error.
std::unique_ptr<ast::TranslationUnit> Parse(const char* path) {
    yyin = std::fopen(path, "r");
    if (!yyin) return nullptr;
    std::unique_ptr<ast::TranslationUnit> root;
    int status = yyparse(root);
    std::fclose(yyin);
    yylex_destroy();
    return status == 0 ? std::move(root) : nullptr;
}

}  // namespace

int main(int argc, char** argv) {
    Options opts;
    if (!ParseArgs(argc, argv, opts)) {
        std::cerr << "usage: c_compiler -S in.c -o out.s\n";
        return 1;
    }
    std::unique_ptr<ast::TranslationUnit> root = Parse(opts.input);
    if (!root) return 1;

    std::ofstream out(opts.output);
    Context ctx;
    try {
        root->EmitRISC(out, ctx);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
