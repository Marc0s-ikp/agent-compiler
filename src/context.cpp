#include "context.hpp"

#include "const_value.hpp"

#include <cstdint>
#include <stdexcept>

Context::~Context() {
    for (const TypePtr& record : records_) Type::ReleaseRecord(record);
}

TypePtr Context::NewRecord(bool is_union, const std::string& tag) {
    records_.push_back(Type::Record(is_union, tag));
    return records_.back();
}

void Context::PushScope() {
    scopes_.emplace_back();
    tag_scopes_.emplace_back();
}

void Context::PopScope() {
    scopes_.pop_back();
    tag_scopes_.pop_back();
}

void Context::DeclareTag(const std::string& tag, const TypePtr& type) {
    tag_scopes_.back()[tag] = type;
}

TypePtr Context::FindTag(const std::string& tag) const {
    for (auto scope = tag_scopes_.rbegin(); scope != tag_scopes_.rend();
         ++scope) {
        auto found = scope->find(tag);
        if (found != scope->end()) return found->second;
    }
    return nullptr;
}

TypePtr Context::FindTagHere(const std::string& tag) const {
    auto found = tag_scopes_.back().find(tag);
    return found == tag_scopes_.back().end() ? nullptr : found->second;
}

TypePtr Context::LookupTag(const std::string& tag) const {
    TypePtr type = FindTag(tag);
    if (!type) throw std::runtime_error("unknown tag '" + tag + "'");
    return type;
}

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
    return_type_ = return_type;
}

std::string Context::NewLabel(const std::string& prefix) {
    return prefix + std::to_string(label_count_++);
}

std::string Context::NewSymbolName(const std::string& prefix) {
    return prefix + "." + std::to_string(label_count_++);
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

Context::GlobalVar& Context::FindGlobal(const std::string& label) {
    for (GlobalVar& g : globals_) {
        if (g.name == label) return g;
    }
    throw std::runtime_error("'" + label + "' is not a global");
}

void Context::DeclareGlobal(const std::string& name, const TypePtr& type,
                            bool is_extern) {
    Symbol& slot = scopes_.front()[name];
    if (!slot.type) {
        slot = Symbol::Global(type, name);
        globals_.push_back(GlobalVar{name, type, false, !is_extern, false, {}, false});
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

void Context::InitGlobal(const std::string& name, const std::vector<ir::DataItem>& items) {
    GlobalVar& var = FindGlobal(name);
    if (var.initialised) {
        throw std::runtime_error("redefinition of '" + name + "'");
    }
    var.initialised = true;
    var.has_storage = true;
    var.items = items;
}

std::string Context::AddStaticLocal(const TypePtr& type) {
    std::string label = NewSymbolName("static");
    globals_.push_back(GlobalVar{label, type, true, true, false, {}, true});
    return label;
}

void Context::CompleteStaticLocal(const std::string& label, const TypePtr& type,
                                  const std::vector<ir::DataItem>& items) {
    GlobalVar& var = FindGlobal(label);
    var.type = type;
    var.initialised = true;
    var.items = items;
}

void Context::MakeGlobalInternal(const std::string& name) {
    FindGlobal(name).internal = true;
}

void Context::ExportGlobals(ir::Module& module) const {
    for (const GlobalVar& g : globals_) {
        if (!g.has_storage) {
            module.external_data.insert(g.name);
            continue;
        }
        if (g.type->Size() == 0) throw std::runtime_error("'" + g.name + "' has incomplete type");
        ir::DataEntry entry;
        entry.label = g.name;
        entry.size = g.type->Size();
        entry.align = g.type->Align();
        entry.internal = g.internal;
        entry.compiler_made = g.compiler_made;
        if (g.initialised) entry.items = g.items;  // else zero-filled: no items
        module.data.push_back(entry);
    }
    ExportStrings(module);
    ExportFloats(module);
}

// Each pooled string is a read-only, compiler-made entry: its bytes and the NUL.
void Context::ExportStrings(ir::Module& module) const {
    for (const auto& [label, bytes] : strings_) {
        ir::DataItem item;
        item.kind = ir::DataItem::Kind::Bytes;
        item.bytes = bytes + std::string(1, '\0');
        ir::DataEntry entry;
        entry.label = label;
        entry.size = static_cast<int>(item.bytes.size());
        entry.align = 1;
        entry.read_only = true;
        entry.internal = true;
        entry.compiler_made = true;
        entry.items.push_back(item);
        module.data.push_back(entry);
    }
}

// Each pooled float constant is a read-only, compiler-made entry of one Float
// item, aligned to its size (21.2a).
void Context::ExportFloats(ir::Module& module) const {
    for (const FloatConst& f : floats_) {
        ir::DataItem item;
        item.kind = ir::DataItem::Kind::Float;
        item.size = f.size;
        item.value = static_cast<long long>(f.bits);
        ir::DataEntry entry;
        entry.label = f.label;
        entry.size = f.size;
        entry.align = f.size;
        entry.read_only = true;
        entry.internal = true;
        entry.compiler_made = true;
        entry.items.push_back(item);
        module.data.push_back(entry);
    }
}

void Context::PushSwitch(const std::string& break_label) {
    break_labels_.push_back(break_label);
    switches_.emplace_back();
}

Context::SwitchInfo Context::PopSwitch() {
    break_labels_.pop_back();
    SwitchInfo info = std::move(switches_.back());
    switches_.pop_back();
    return info;
}

// Case values are compared as 32-bit integers, so they are wrapped to that
// width first; two labels that wrap to the same value are duplicates.
std::string Context::AddCase(long value) {
    if (switches_.empty()) throw std::runtime_error("case outside switch");
    SwitchInfo& info = switches_.back();
    long wrapped = ast::WrapToType(value, *Type::Int());
    for (const auto& c : info.cases) {
        if (c.first == wrapped) throw std::runtime_error("duplicate case value");
    }
    std::string label = NewLabel("case");
    info.cases.emplace_back(wrapped, label);
    return label;
}

std::string Context::AddDefault() {
    if (switches_.empty()) throw std::runtime_error("default outside switch");
    SwitchInfo& info = switches_.back();
    if (!info.default_label.empty()) {
        throw std::runtime_error("duplicate default label");
    }
    info.default_label = NewLabel("default");
    return info.default_label;
}

std::string Context::AddString(const std::string& bytes) {
    for (const auto& s : strings_) {
        if (s.second == bytes) return s.first;
    }
    strings_.emplace_back(NewSymbolName("str"), bytes);
    return strings_.back().first;
}

std::string Context::AddFloat(uint64_t bits, int size) {
    for (const FloatConst& f : floats_) {
        if (f.bits == bits && f.size == size) return f.label;
    }
    floats_.push_back({NewSymbolName("flt"), bits, size});
    return floats_.back().label;
}
