#pragma once
#include <string>
#include <vector>

#include "ast_declaration.hpp"
#include "ast_node.hpp"

namespace ast {

// One piece of an initialised object: either a scalar given by an expression,
// a whole struct given by an expression of that struct type, or the bytes of
// a string literal that initialises a char array.
struct InitItem {
    int offset;             // byte offset inside the object
    TypePtr type;           // the scalar or struct type, or the char array type
    const Expr* value;      // the scalar's or struct's expression; null for a string
    std::string bytes;      // string only: the chars to store, without NUL
};

// An initialiser laid out against its type. Items are in increasing offset
// order and never overlap; every byte not covered by an item is zero. An
// array declared without a length takes it from the initialiser, so 'type'
// is the completed type.
struct FlatInit {
    TypePtr type;
    std::vector<InitItem> items;
};

// Applies the C rules for initialisers once, for globals, static locals and
// ordinary locals alike: nested, partial and flat (brace-elided) lists, a
// string literal for a char array (bare or in braces), and '{x}' for a scalar.
// Structs and unions take their members in order (a union only its first),
// with the same nesting, partial lists and brace elision; a struct-valued
// expression initialises a whole struct. The context gives the types of
// expressions; nothing is evaluated or emitted. Throws std::runtime_error for an
// initialiser that does not fit the type.
FlatInit FlattenInit(const TypePtr& type, const Initializer& init,
                     const Context& ctx);

}  // namespace ast
