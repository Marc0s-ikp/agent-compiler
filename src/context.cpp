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

namespace {
bool SameTypes(const std::vector<TypePtr>& a, const std::vector<TypePtr>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (!a[i]->Equals(*b[i])) return false;
    }
    return true;
}
}  // namespace

// Parameters are compared only when both declarations list some, because
// 'int f();' leaves them unspecified.
void Context::DeclareFunction(const std::string& name,
                              const TypePtr& return_type,
                              const std::vector<TypePtr>& params) {
    Symbol& slot = scopes_.front()[name];
    if (!slot.type) {
        slot = Symbol{return_type, 0, SymbolKind::Function, params};
        return;
    }
    bool clash = slot.kind != SymbolKind::Function ||
                 !slot.type->Equals(*return_type) ||
                 (!slot.params.empty() && !params.empty() &&
                  !SameTypes(slot.params, params));
    if (clash) throw std::runtime_error("conflicting declaration of '" + name + "'");
    if (slot.params.empty()) slot.params = params;
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
