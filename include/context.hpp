#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "ir.hpp"
#include "type.hpp"

enum class SymbolKind { Local, Global, Function, EnumConst, Typedef };

// A named entity.
// For a Local, 'object' is the name of its frame object in the IR function.
// For a Typedef, 'type' is the type the name stands for.
// For an EnumConst, 'value' is its value (type int).
// For a Global, 'label' is the name of its data entry.
// For a Function, 'type' is the Function type (return type and parameters)
// and no storage is recorded (D9).
struct Symbol {
    TypePtr type;
    SymbolKind kind = SymbolKind::Local;
    std::string label;
    long value = 0;
    std::string object;

    static Symbol Local(TypePtr type, std::string object) {
        return Symbol{std::move(type), SymbolKind::Local, "", 0, std::move(object)};
    }
    static Symbol Global(TypePtr type, std::string label) {
        return Symbol{std::move(type), SymbolKind::Global, std::move(label), 0, ""};
    }
    static Symbol Typedef(TypePtr type) {
        return Symbol{std::move(type), SymbolKind::Typedef, "", 0, ""};
    }
    static Symbol EnumConst(long value) {
        return Symbol{Type::Int(), SymbolKind::EnumConst, "", value, ""};
    }
    static Symbol Function(TypePtr type) {
        return Symbol{std::move(type), SymbolKind::Function, "", 0, ""};
    }
};

// Front-end state: scopes, the globals and constant pools, the control-flow
// targets of the current function, and label generation. Frame layout and
// registers belong to the backend (D21).
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
    // the initialiser as data items. ExportGlobals hands all storage to the
    // module once, after the functions.
    void DeclareGlobal(const std::string& name, const TypePtr& type,
                       bool is_extern);
    void InitGlobal(const std::string& name, const std::vector<ir::DataItem>& items);
    // A 'static' local: storage under a fresh label, never exported. Returns
    // the label. It is zero-filled until CompleteStaticLocal gives it its
    // final type (an unsized array) and initialiser items.
    std::string AddStaticLocal(const TypePtr& type);
    void CompleteStaticLocal(const std::string& label, const TypePtr& type,
                             const std::vector<ir::DataItem>& items);
    // Marks an already declared global as having internal linkage.
    void MakeGlobalInternal(const std::string& name);
    // One data entry per object that has storage (21.2a), and the extern-only
    // names as module.external_data; also the string and float pools.
    void ExportGlobals(ir::Module& module) const;

    // String literals (read-only data). The same bytes get the same label, so
    // evaluating one literal twice adds nothing. A NUL is written after them.
    std::string AddString(const std::string& bytes);

    // Floating constants (read-only data): the IEEE bit pattern of a float
    // (size 4) or double (size 8). The same bits and size get the same label.
    std::string AddFloat(uint64_t bits, int size);

    // Starts a function: records its return type for 'return'.
    void BeginFunction(const TypePtr& return_type);

    // Control. A loop sets both targets; a switch sets only break, so 'continue' inside a switch still finds the enclosing loop.
    void PushLoop(const std::string& break_label,
                  const std::string& continue_label);
    void PopLoop();
    const std::string& BreakLabel() const;
    const std::string& ContinueLabel() const;

    // Switch. The dispatch is written after the body, so each case registers
    // its value here and gets a label back.
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
    const TypePtr& ReturnType() const { return return_type_; }

    // Labels are unique across the whole file and target-neutral: a code
    // label is "else3", a compiler-made data symbol (string, float, static
    // local) is "str.3". The dot cannot appear in a C name, so a data symbol
    // never clashes with one. The backend alone decides how they are spelled
    // in assembler.
    std::string NewLabel(const std::string& prefix);

private:
    std::string NewSymbolName(const std::string& prefix);  // "str.3"

    struct GlobalVar {
        std::string name;          // the label
        TypePtr type;
        bool internal = false;     // 'static': no .globl
        bool has_storage = false;  // defined here, not only declared extern
        bool initialised = false;
        std::vector<ir::DataItem> items;  // the initialiser
        bool compiler_made = false;       // a static local's label
    };
    GlobalVar& FindGlobal(const std::string& label);
    void ExportStrings(ir::Module& module) const;
    void ExportFloats(ir::Module& module) const;

    std::vector<std::map<std::string, Symbol>> scopes_;
    std::vector<std::map<std::string, TypePtr>> tag_scopes_;
    std::vector<TypePtr> records_;
    std::vector<std::string> break_labels_;
    std::vector<std::string> continue_labels_;
    std::vector<SwitchInfo> switches_;
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
};
