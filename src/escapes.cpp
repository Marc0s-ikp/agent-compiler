#include "escapes.hpp"

#include <cctype>
#include <stdexcept>

namespace {

bool IsOctal(char c) { return c >= '0' && c <= '7'; }

// The byte for a one-letter escape such as the 'n' of \n.
char SimpleEscape(char c) {
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'a': return '\a';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'v': return '\v';
        case '\\': case '\'': case '"': case '?': return c;
    }
    throw std::runtime_error(std::string("unknown escape sequence '\\") + c +
                             "'");
}

// Reads up to three octal digits starting at body[i]; advances i past them.
char OctalEscape(const std::string& body, size_t& i) {
    int value = 0;
    for (int n = 0; n < 3 && i < body.size() && IsOctal(body[i]); ++n, ++i) {
        value = value * 8 + (body[i] - '0');
    }
    return static_cast<char>(value & 0xff);
}

// Reads \x digits starting at body[i] (just after the 'x'); advances i.
char HexEscape(const std::string& body, size_t& i) {
    if (i >= body.size() || !std::isxdigit(static_cast<unsigned char>(body[i]))) {
        throw std::runtime_error("\\x used with no following hex digits");
    }
    int value = 0;
    for (; i < body.size() && std::isxdigit(static_cast<unsigned char>(body[i]));
         ++i) {
        char d = body[i];
        int digit = std::isdigit(static_cast<unsigned char>(d))
                        ? d - '0'
                        : std::tolower(d) - 'a' + 10;
        value = (value * 16 + digit) & 0xff;
    }
    return static_cast<char>(value);
}

}  // namespace

std::string DecodeEscapes(const std::string& body) {
    std::string out;
    size_t i = 0;
    while (i < body.size()) {
        if (body[i] != '\\') {
            out += body[i++];
            continue;
        }
        if (++i >= body.size()) throw std::runtime_error("dangling backslash");
        if (IsOctal(body[i])) {
            out += OctalEscape(body, i);
        } else if (body[i] == 'x') {
            ++i;
            out += HexEscape(body, i);
        } else {
            out += SimpleEscape(body[i++]);
        }
    }
    return out;
}

std::string AsmQuoted(const std::string& bytes) {
    std::string out = "\"";
    for (unsigned char c : bytes) {
        if (c >= 32 && c < 127 && c != '"' && c != '\\') {
            out += static_cast<char>(c);
            continue;
        }
        out += '\\';
        out += static_cast<char>('0' + (c >> 6));
        out += static_cast<char>('0' + ((c >> 3) & 7));
        out += static_cast<char>('0' + (c & 7));
    }
    return out + "\"";
}
