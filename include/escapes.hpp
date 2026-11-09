#pragma once
#include <string>

// The text between the quotes of a character or string literal, with every
// escape sequence replaced by the byte it stands for: \n \t etc., \\ \' \" \?,
// up to three octal digits, and \x followed by any number of hex digits
// (the value is taken modulo 256). The result may hold NUL or bytes above 127.
// Throws std::runtime_error for a malformed escape.
std::string DecodeEscapes(const std::string& body);

// The reverse, for the assembler: 'bytes' as a quoted string operand for
// .ascii. Printable characters stand for themselves; everything else
// (including NUL, quotes, backslash and bytes above 127) is a 3-digit octal
// escape, which can never swallow a following digit.
std::string AsmQuoted(const std::string& bytes);
