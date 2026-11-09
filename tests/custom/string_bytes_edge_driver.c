int hi_bytes(void);
int hi_size(void);
int quote_slash(void);
int offsets(void);
int static_ptr(void);
int static_local_str(void);
int octal_then_digit(void);
int hex_then_end(void);

int main()
{
    if (hi_bytes() != 255 * 1000 + 128 - 0 + 'z') return 1;
    if (hi_size() != 7) return 2;
    if (quote_slash() != '"' * 1000 + '\\') return 3;
    if (offsets() != 'e' * 1000 + 'r') return 4;
    if (static_ptr() != 's') return 5;
    if (static_local_str() != 'b' * 1000 + 'z' + 4) return 6;
    if (octal_then_digit() != '1') return 7;
    if (hex_then_end() != 'A') return 8; /* "\x41" "B" is "AB" */
    return 0;
}
