#pragma once
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "type.hpp"

enum class SymbolKind { Local, Global, Function, EnumConst, Typedef };

// A named entity. For a Local, 'offset' is relative to s0 (negative: locals
// and temps, zero or positive: parameters passed on the caller's stack).
// For a Typedef, 'type' is the type the name stands for.
// For an EnumConst, 'value' is its value (type int).
// For a Global, 'label' is the assembly name of its storage.
// For a Function, 'type' is the Function type (return type and parameters)
// and no storage is recorded (D9).
struct Symbol {
    TypePtr type;
    int offset = 0;
    SymbolKind kind = SymbolKind::Local;
    std::string label;
    long value = 0;

    static Symbol Local(TypePtr type, int offset) {
        return Symbol{std::move(type), offset, SymbolKind::Local, "", 0};
    }
    static Symbol Global(TypePtr type, std::string label) {
        return Symbol{std::move(type), 0, SymbolKind::Global, std::move(label), 0};
    }
    static Symbol Typedef(TypePtr type) {
        return Symbol{std::move(type), 0, SymbolKind::Typedef, "", 0};
    }
    static Symbol EnumConst(long value) {
        return Symbol{Type::Int(), 0, SymbolKind::EnumConst, "", value};
    }
    static Symbol Function(TypePtr type) {
        return Symbol{std::move(type), 0, SymbolKind::Function, "", 0};
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
    // Struct, union and enum tags live in their own namespace, scoped like
    // ordinary names. Declare in the innermost scope; Lookup searches outwards.
    void DeclareTag(const std::string& tag, const TypePtr& type);
    TypePtr LookupTag(const std::string& tag) const;  // throws if unknown
    // Record a function in file scope; throws if it clashes with an earlier
    // declaration of the same name.
    void DeclareFunction(const std::string& name, const TypePtr& function_type);

    // Globals (D11). DeclareGlobal records the symbol; an 'extern' declaration
    // alone reserves no storage. A later definition of the same name (with or
    // without an initialiser) is merged into one object. InitGlobal supplies
    // the initialiser as assembly data directives. EmitGlobals writes all
    // storage once, after the functions.
    void DeclareGlobal(const std::string& name, const TypePtr& type,
                       bool is_extern);
    void InitGlobal(const std::string& name, const std::string& data);
    // A 'static' local: storage under a fresh label, never exported. Returns
    // the label; 'data' is the initialiser directives, or null for zeros.
    std::string AddStaticLocal(const TypePtr& type, const std::string* data);
    // Marks an already declared global as having internal linkage.
    void MakeGlobalInternal(const std::string& name);
    void EmitGlobals(std::ostream& os) const;

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
    struct GlobalVar {
        std::string name;          // the label
        TypePtr type;
        bool internal = false;     // 'static': no .globl
        bool has_storage = false;  // defined here, not only declared extern
        bool initialised = false;
        std::string data;          // initialiser directives
    };
    GlobalVar& FindGlobal(const std::string& label);

    int Allocate(const TypePtr& type);

    std::vector<std::map<std::string, Symbol>> scopes_;
    std::vector<std::map<std::string, TypePtr>> tag_scopes_;
    int used_ = 0;                  // bytes currently allocated below s0
    int max_used_ = 0;              // high-water mark, decides the frame size
    int outgoing_ = 0;              // outgoing stack-argument area (feature 3)
    std::vector<int> temp_marks_;   // 'used_' before each live temp
    std::vector<std::string> break_labels_;
    std::vector<std::string> continue_labels_;
    std::string return_label_;
    TypePtr return_type_;
    std::vector<GlobalVar> globals_;
    int label_count_ = 0;
};
