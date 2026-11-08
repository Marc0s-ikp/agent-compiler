#pragma once
#include <map>
#include <string>
#include <vector>

// Parse-time scoped table that tells the lexer whether an identifier is a
// type name (D7). It exists before Context does, and is updated by the
// parser's declaration actions:
//   - a typedef declarator declares its name as a type in the current scope;
//   - any other declarator (variable, function, parameter, enumerator)
//     declares the name as an ordinary identifier, hiding an outer typedef;
//   - '{' opens a scope and '}' closes it, so a hiding declaration ends with
//     its block and the typedef is visible again.
// Parameters are seen before the '{' of the function body, so they wait in a
// pending list and enter the body's scope when it opens.
class TypedefTable {
public:
    TypedefTable() { scopes_.emplace_back(); }

    void PushScope();  // also moves the pending parameters into it
    void PopScope();
    void Declare(const std::string& name, bool is_typedef);
    bool IsTypeName(const std::string& name) const;

    void AddPendingParam(const std::string& name) { pending_.push_back(name); }
    void ClearPending() { pending_.clear(); }

    // True while the declarators of a 'typedef ...' declaration are parsed.
    void SetInTypedef(bool on) { in_typedef_ = on; }
    bool InTypedef() const { return in_typedef_; }

private:
    std::vector<std::map<std::string, bool>> scopes_;  // name -> is a type
    std::vector<std::string> pending_;
    bool in_typedef_ = false;
};

// The one table shared by the lexer and the parser.
TypedefTable& Typedefs();
