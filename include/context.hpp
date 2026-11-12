#pragma once
#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "ir.hpp"
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
    // --ir only: the name of the local's frame object (offset is unused).
    std::string object;

    static Symbol IrLocal(TypePtr type, std::string object) {
        Symbol s = Local(std::move(type), 0);
        s.object = std::move(object);
        return s;
    }
    static Symbol Local(TypePtr type, int offset) {
        return Symbol{std::move(type), offset, SymbolKind::Local, "", 0, ""};
    }
    static Symbol Global(TypePtr type, std::string label) {
        return Symbol{std::move(type), 0, SymbolKind::Global, std::move(label), 0, ""};
    }
    static Symbol Typedef(TypePtr type) {
        return Symbol{std::move(type), 0, SymbolKind::Typedef, "", 0, ""};
    }
    static Symbol EnumConst(long value) {
        return Symbol{Type::Int(), 0, SymbolKind::EnumConst, "", value, ""};
    }
    static Symbol Function(TypePtr type) {
        return Symbol{std::move(type), 0, SymbolKind::Function, "", 0, ""};
    }
};

// Code generation state: scopes, the stack frame of the current function,
// and label generation. See design sections 3 and 4.
class Context {
public:
    Context() = default;
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    // Releases the members of every record made by NewRecord, which break
    // the reference cycles of self-referential records.
    ~Context();

    // A new, incomplete struct or union, owned (for cleanup) by this context.
    TypePtr NewRecord(bool is_union, const std::string& tag);

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
    TypePtr FindTag(const std::string& tag) const;    // null if unknown
    // Only the innermost scope: a tag declared there, else null.
    TypePtr FindTagHere(const std::string& tag) const;
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
    // 'data' is the old path's directives, 'items' the same bytes for the IR.
    void InitGlobal(const std::string& name, const std::string& data,
                    const std::vector<ir::DataItem>& items);
    // A 'static' local: storage under a fresh label, never exported. Returns
    // the label. It is zero-filled until CompleteStaticLocal gives it its
    // final type (an unsized array) and initialiser directives.
    std::string AddStaticLocal(const TypePtr& type);
    void CompleteStaticLocal(const std::string& label, const TypePtr& type,
                             const std::string& data,
                             const std::vector<ir::DataItem>& items);
    // Marks an already declared global as having internal linkage.
    void MakeGlobalInternal(const std::string& name);
    void EmitGlobals(std::ostream& os) const;
    // The --ir twin of EmitGlobals: one data entry per object that has storage
    // (21.2a), and the extern-only names as module.external_data.
    void ExportGlobals(ir::Module& module) const;  // also the string pool

    // String literals (read-only data). The same bytes get the same label, so
    // evaluating one literal twice adds nothing. A NUL is written after them.
    std::string AddString(const std::string& bytes);
    void EmitStrings(std::ostream& os) const;

    // Floating constants (read-only data): the IEEE bit pattern of a float
    // (size 4) or double (size 8). The same bits and size get the same label.
    std::string AddFloat(uint64_t bits, int size);
    void EmitFloats(std::ostream& os) const;

    // Frame. Slots grow downwards from s0, below the saved ra and s0.
    void BeginFunction(const TypePtr& return_type);
    // A slot that lasts until the function ends. PopTemp never releases it,
    // even if it was allocated while temps were live (D19).
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

    // Switch. The dispatch is written after the body (the body is buffered),
    // so each case registers its value here and gets a label back.
    // 'break' finds the switch through the same stack as loops; 'continue'
    // does not, because only PushLoop sets a continue target.
    struct SwitchInfo {
        std::vector<std::pair<long, std::string>> cases;  // value, label
        std::string default_label;                        // empty if none
    };
    void PushSwitch(const std::string& break_label);
    SwitchInfo PopSwitch();
    // Both throw outside a switch, or for a duplicate case / second default.
    std::string AddCase(long value);
    std::string AddDefault();

    // Function-level information.
    const std::string& ReturnLabel() const { return return_label_; }
    const TypePtr& ReturnType() const { return return_type_; }
    // Where a function returning a struct through a hidden pointer keeps that
    // pointer (an s0 offset); set by the prologue code, read by 'return'.
    void SetReturnPointerSlot(int offset) { return_pointer_slot_ = offset; }
    int ReturnPointerSlot() const { return return_pointer_slot_; }

    // Labels are unique across the whole file. On the old path they are
    // assembler syntax (".Lelse3"). After UseNeutralNames (the IR path) they are
    // target-neutral: a code label is "else3", a compiler-made data symbol
    // (string, float, static local) is "str.3". The dot cannot appear in a C
    // name, so a data symbol never clashes with one. The IR backend alone
    // decides how they are spelled in assembler.
    std::string NewLabel(const std::string& prefix);
    std::string NewSymbolName(const std::string& prefix);
    void UseNeutralNames() { neutral_names_ = true; }

private:
    struct GlobalVar {
        std::string name;          // the label
        TypePtr type;
        bool internal = false;     // 'static': no .globl
        bool has_storage = false;  // defined here, not only declared extern
        bool initialised = false;
        std::string data;          // initialiser directives
        std::vector<ir::DataItem> items;  // the same, for the IR
        bool compiler_made = false;       // a static local's label
    };
    GlobalVar& FindGlobal(const std::string& label);
    void ExportStrings(ir::Module& module) const;
    void ExportFloats(ir::Module& module) const;

    int Allocate(const TypePtr& type);

    std::vector<std::map<std::string, Symbol>> scopes_;
    std::vector<std::map<std::string, TypePtr>> tag_scopes_;
    std::vector<TypePtr> records_;
    int used_ = 0;                  // bytes currently allocated below s0
    int max_used_ = 0;              // high-water mark, decides the frame size
    int floor_ = 0;                 // bytes that PopTemp must never release
    int return_pointer_slot_ = 0;
    int outgoing_ = 0;              // outgoing stack-argument area (feature 3)
    std::vector<int> temp_marks_;   // 'used_' before each live temp
    std::vector<std::string> break_labels_;
    std::vector<std::string> continue_labels_;
    std::vector<SwitchInfo> switches_;
    std::string return_label_;
    TypePtr return_type_;
    std::vector<GlobalVar> globals_;
    std::vector<std::pair<std::string, std::string>> strings_;  // label, bytes
    struct FloatConst {
        std::string label;
        uint64_t bits;
        int size;
    };
    std::vector<FloatConst> floats_;
    int label_count_ = 0;
    bool neutral_names_ = false;
};
