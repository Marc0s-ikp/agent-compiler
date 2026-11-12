#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "backend.hpp"
#include "ir.hpp"
#include "ir_gen.hpp"
#include "target.hpp"


extern FILE* yyin;
int yylex_destroy();
int yyparse(std::unique_ptr<ast::TranslationUnit>& root);

namespace {

struct Options {
    const char* input = nullptr;
    const char* output = nullptr;
    const char* target = "riscv32";  // the default when --target is absent
    bool use_ir = false;             // --ir: AST -> IR -> RISC-V backend
    bool emit_ir = false;            // --emit-ir: write the IR as text
};

bool ParseArgs(int argc, char** argv, Options& opts) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-S") == 0) continue;
        if (std::strcmp(argv[i], "--target") == 0 && i + 1 >= argc) {
            std::cerr << "error: --target needs a value\n";
            return false;
        }
        if (std::strcmp(argv[i], "--ir") == 0) {
            opts.use_ir = true;
        } else if (std::strcmp(argv[i], "--emit-ir") == 0) {
            opts.emit_ir = true;
        } else if (std::strcmp(argv[i], "--target") == 0 && i + 1 < argc) {
            opts.target = argv[++i];
        } else if (std::strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
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

// Prints the checker's messages; returns true if there were any.
bool ReportIRErrors(const ir::Module& module) {
    std::vector<std::string> errors = ir::CheckIR(module);
    for (const std::string& e : errors) std::cerr << "ir check: " << e << "\n";
    return !errors.empty();
}

// The IR path. The checker runs on every compile, in every build (D21).
// With --emit-ir the dump is written FIRST and the checker runs after it, so a
// broken module can still be read. If lowering throws, the finished functions
// and the partly built one are dumped before the error is reported. With --ir
// alone the checker runs before
// the backend, and nothing is generated from a module that fails it.
int CompileViaIR(const Options& opts, const ast::TranslationUnit& root, std::ostream& out) {
    ir::Module module;
    Context ctx;
    ctx.UseNeutralNames();  // the backend spells labels (21.2a)
    IRGen gen(module, ctx);
    try {
        root.EmitIR(gen);
    } catch (const std::exception&) {
        if (!opts.emit_ir) throw;
        gen.AppendPartialFunction();  // D23 c: dump what was built, then fail
        ir::Print(module, out);
        throw;
    }
    if (opts.emit_ir) {
        ir::Print(module, out);          // 1. the dump
        return ReportIRErrors(module) ? 1 : 0;  // 2. then the checker
    }
    if (ReportIRErrors(module)) return 1;  // checker first, then the backend
    EmitRiscv(module, out);
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    Options opts;
    if (!ParseArgs(argc, argv, opts)) {
        std::cerr << "usage: c_compiler -S in.c -o out.s\n";
        return 1;
    }
    TargetInfo target;
    if (!FindTarget(opts.target, target)) {
        std::cerr << "error: unknown target '" << opts.target << "'\n";
        return 1;
    }
    SetTarget(target);  // before parsing: types read Target()
    std::unique_ptr<ast::TranslationUnit> root = Parse(opts.input);
    if (!root) return 1;

    std::ofstream out(opts.output);
    try {
        if (opts.use_ir || opts.emit_ir) return CompileViaIR(opts, *root, out);
        Context ctx;
        root->EmitRISC(out, ctx);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
