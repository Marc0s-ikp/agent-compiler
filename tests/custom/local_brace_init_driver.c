int partial_dirty(void);
int inferred(void);
int exprs(int x);
int nested(void);
int nested_partial_dirty(void);
int scalar_braces(void);
int char_list(void);
int zero_all_dirty(void);
int trailing_comma(void);

int main()
{
    if (partial_dirty() != 120) return 1;
    if (inferred() != 12456) return 2;
    if (exprs(3) != 300 + 40 + 6) return 3;
    if (nested() != 1234) return 4;
    if (nested_partial_dirty() != 1 + 20 + 300) return 5;
    if (scalar_braces() != 7) return 6;
    if (char_list() != 'a' + 'b') return 7;
    if (zero_all_dirty() != 0) return 8;
    if (trailing_comma() != 6) return 9;
    return 0;
}
