#pragma once
#include <set>
#include <string>

#include "ir.hpp"

// The one place both backends spell IR names as assembler names.
// An IR code label ("else3") and a compiler-made data symbol ("str.0") become
// local assembler labels with the ".L" prefix, which stay out of the symbol
// table. Compiler-made data (strings, float constants, static locals) has a
// neutral name with a '.', which no C name has. A C-named global or function
// keeps its name.
namespace backend {

inline std::string LocalLabel(const std::string& name) { return ".L" + name; }

class SymbolNames {
public:
    explicit SymbolNames(const ir::Module& module) {
        for (const ir::DataEntry& d : module.data) if (d.compiler_made) local_.insert(d.label);
    }
    std::string Spell(const std::string& name) const {
        return local_.count(name) ? LocalLabel(name) : name;
    }

private:
    std::set<std::string> local_;
};

}  // namespace backend
