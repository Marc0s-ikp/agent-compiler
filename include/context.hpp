#pragma once
#include <map>
#include <string>
#include <vector>

#include "type.hpp"

enum class SymbolKind { Local, Function };

// A named entity. For a Local, 'offset' is relative to s0 (negative: locals
// and temps, zero or positive: parameters passed on the caller's stack).
// For a Function, 'type' is the Function type (return type and parameters)
// and no storage is recorded (D9).
struct Symbol {
    TypePtr type;
    int offset = 0;
    SymbolKind kind = SymbolKind::Local;

    static Symbol Local(TypePtr type, int offset) {
        return Symbol{std::move(type), offset, SymbolKind::Local};
    }
    static Symbol Function(TypePtr type) {
        return Symbol{std::move(type), 0, SymbolKind::Function};
    }
};

// Code generation state: scopes, the stack frame of the current function,
// and label generation. See design sections 3 and 4.
class Context {
public:
    // Scopes. Lookup searches the innermost scope first.
    void PushScope();
    void PopScope();
    void Declare(const std::string& name, const Symbol& symbol);
    const Symbol& Lookup(const std::string& name) const;  // throws if unknown
    const Symbol* Find(const std::string& name) const;    // null if unknown
    // Record a function in file scope; throws if it clashes with an earlier
    // declaration of the same name.
    void DeclareFunction(const std::string& name, const TypePtr& function_type);

    // Frame. Slots grow downwards from s0, below the saved ra and s0.
    void BeginFunction(const TypePtr& return_type);
    int AllocLocal(const TypePtr& type);
    int PushTemp(const TypePtr& type);
    void PopTemp();
    // Make room for bytes of outgoing stack arguments at sp (design section 4).
    void ReserveOutgoing(int bytes);
    int FrameSize() const;

    // Control. A loop sets both targets; a switch (feature 5) will set only
    // break, so 'continue' inside a switch still finds the enclosing loop.
    void PushLoop(const std::string& break_label,
                  const std::string& continue_label);
    void PopLoop();
    const std::string& BreakLabel() const;
    const std::string& ContinueLabel() const;

    // Function-level information.
    const std::string& ReturnLabel() const { return return_label_; }
    const TypePtr& ReturnType() const { return return_type_; }

    // Labels are unique across the whole file.
    std::string NewLabel(const std::string& prefix);

private:
    int Allocate(const TypePtr& type);

    std::vector<std::map<std::string, Symbol>> scopes_;
    int used_ = 0;                  // bytes currently allocated below s0
    int max_used_ = 0;              // high-water mark, decides the frame size
    int outgoing_ = 0;              // outgoing stack-argument area (feature 3)
    std::vector<int> temp_marks_;   // 'used_' before each live temp
    std::vector<std::string> break_labels_;
    std::vector<std::string> continue_labels_;
    std::string return_label_;
    TypePtr return_type_;
    int label_count_ = 0;
};
