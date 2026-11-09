int big_int_dirty(void);
int big_char_dirty(void);
int unaligned_char_dirty(void);
int string_in_braces(void);
int inferred_flat(void);
char *ptr_init(void);

int main()
{
    if (big_int_dirty() != 116) return 1;
    if (big_char_dirty() != 'x' + 'y') return 2;
    if (unaligned_char_dirty() != 10) return 3;
    if (string_in_braces() != 3000 + 'i') return 4;
    if (inferred_flat() != 300 + 50) return 5;
    if (ptr_init()[2] != 'r') return 6;
    return 0;
}
