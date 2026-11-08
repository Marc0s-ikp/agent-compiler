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

const Symbol& Context::Lookup(const std::string& name) const {
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
        auto found = scope->find(name);
        if (found != scope->end()) return found->second;
    }
    throw std::runtime_error("undeclared identifier '" + name + "'");
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
int Context::FrameSize() const { return RoundUp(max_used_ + outgoing_, 16); }

std::string Context::NewLabel(const std::string& prefix) {
    return ".L" + prefix + std::to_string(label_count_++);
}
