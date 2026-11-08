#include "context.hpp"

#include <stdexcept>

namespace {
const int kSavedBytes = 8;  // ra and the caller's s0

int RoundUp(int value, int multiple) {
    return (value + multiple - 1) / multiple * multiple;
}
}  // namespace

void Context::PushScope() { scopes_.emplace_back(); }

void Context::PopScope() { scopes_.pop_back(); }

void Context::Declare(const std::string& name, const Symbol& symbol) {
    scopes_.back()[name] = symbol;
}

const Symbol* Context::Find(const std::string& name) const {
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
        auto found = scope->find(name);
        if (found != scope->end()) return &found->second;
    }
    return nullptr;
}

const Symbol& Context::Lookup(const std::string& name) const {
    const Symbol* symbol = Find(name);
    if (!symbol) throw std::runtime_error("undeclared identifier '" + name + "'");
    return *symbol;
}

// Parameters are compared only when both declarations list some, because
// 'int f();' leaves them unspecified.
void Context::DeclareFunction(const std::string& name,
                              const TypePtr& function_type) {
    Symbol& slot = scopes_.front()[name];
    if (!slot.type) {
        slot = Symbol::Function(function_type);
        return;
    }
    const TypePtr& old = slot.type;
    bool compatible = slot.kind == SymbolKind::Function &&
                      old->Base()->Equals(*function_type->Base());
    if (compatible && !old->Params().empty() &&
        !function_type->Params().empty()) {
        compatible = old->Equals(*function_type);
    }
    if (!compatible) {
        throw std::runtime_error("conflicting declaration of '" + name + "'");
    }
    if (old->Params().empty()) slot.type = function_type;
}

void Context::BeginFunction(const TypePtr& return_type) {
    used_ = kSavedBytes;
    max_used_ = kSavedBytes;
    outgoing_ = 0;
    temp_marks_.clear();
    return_type_ = return_type;
    return_label_ = NewLabel("ret");
}

// Reserve a slot aligned for 'type'; returns its offset from s0 (D12).
int Context::Allocate(const TypePtr& type) {
    used_ = RoundUp(used_ + type->Size(), type->Align());
    if (used_ > max_used_) max_used_ = used_;
    return -used_;
}

int Context::AllocLocal(const TypePtr& type) { return Allocate(type); }

// Temps are a stack: PopTemp releases the most recent PushTemp.
int Context::PushTemp(const TypePtr& type) {
    temp_marks_.push_back(used_);
    return Allocate(type);
}

void Context::PopTemp() {
    used_ = temp_marks_.back();
    temp_marks_.pop_back();
}

// Rounded to 16 including the outgoing area, so sp stays ABI-aligned (D12).
void Context::ReserveOutgoing(int bytes) {
    if (bytes > outgoing_) outgoing_ = bytes;
}

int Context::FrameSize() const { return RoundUp(max_used_ + outgoing_, 16); }

std::string Context::NewLabel(const std::string& prefix) {
    return ".L" + prefix + std::to_string(label_count_++);
}

void Context::PushLoop(const std::string& break_label,
                       const std::string& continue_label) {
    break_labels_.push_back(break_label);
    continue_labels_.push_back(continue_label);
}

void Context::PopLoop() {
    break_labels_.pop_back();
    continue_labels_.pop_back();
}

const std::string& Context::BreakLabel() const {
    if (break_labels_.empty()) throw std::runtime_error("break outside loop");
    return break_labels_.back();
}

const std::string& Context::ContinueLabel() const {
    if (continue_labels_.empty()) {
        throw std::runtime_error("continue outside loop");
    }
    return continue_labels_.back();
}

namespace {

// The type two declarations of one global agree on, or null. An array of
// unspecified length ('extern int a[];') is completed by the other.
TypePtr MergeGlobalTypes(const TypePtr& a, const TypePtr& b) {
    if (a->Equals(*b)) return a;
    if (a->IsArray() && b->IsArray() && a->Base()->Equals(*b->Base())) {
        if (a->Length() == 0) return b;
        if (b->Length() == 0) return a;
    }
    return nullptr;
}

}  // namespace

Context::GlobalVar& Context::FindGlobal(const std::string& name) {
    for (GlobalVar& g : globals_) {
        if (g.name == name) return g;
    }
    throw std::runtime_error("'" + name + "' is not a global");
}

void Context::DeclareGlobal(const std::string& name, const TypePtr& type,
                            bool is_extern) {
    Symbol& slot = scopes_.front()[name];
    if (!slot.type) {
        slot = Symbol::Global(type, name);
        globals_.push_back(GlobalVar{name, type, !is_extern, false, ""});
        return;
    }
    TypePtr merged = slot.kind == SymbolKind::Global
                         ? MergeGlobalTypes(slot.type, type)
                         : nullptr;
    if (!merged) {
        throw std::runtime_error("conflicting declaration of '" + name + "'");
    }
    slot.type = merged;
    GlobalVar& var = FindGlobal(name);
    var.type = merged;
    if (!is_extern) var.has_storage = true;
}

void Context::InitGlobal(const std::string& name, const std::string& data) {
    GlobalVar& var = FindGlobal(name);
    if (var.initialised) {
        throw std::runtime_error("redefinition of '" + name + "'");
    }
    var.initialised = true;
    var.has_storage = true;
    var.data = data;
}

void Context::EmitGlobals(std::ostream& os) const {
    for (const GlobalVar& g : globals_) {
        if (!g.has_storage) continue;
        if (g.type->Size() == 0) {
            throw std::runtime_error("'" + g.name + "' has incomplete type");
        }
        os << "\t.globl " << g.name << "\n";
        os << (g.initialised ? "\t.data\n" : "\t.bss\n");
        os << "\t.balign " << g.type->Align() << "\n" << g.name << ":\n";
        if (g.initialised) {
            os << g.data;
        } else {
            os << "\t.zero " << g.type->Size() << "\n";
        }
    }
}
