#include "typedef_table.hpp"

void TypedefTable::PushScope() {
    scopes_.emplace_back();
    for (const std::string& name : pending_) scopes_.back()[name] = false;
    pending_.clear();
}

void TypedefTable::PopScope() { scopes_.pop_back(); }

void TypedefTable::Declare(const std::string& name, bool is_typedef) {
    scopes_.back()[name] = is_typedef;
}

// The innermost declaration of the name decides.
bool TypedefTable::IsTypeName(const std::string& name) const {
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
        auto found = scope->find(name);
        if (found != scope->end()) return found->second;
    }
    return false;
}

TypedefTable& Typedefs() {
    static TypedefTable table;
    return table;
}
