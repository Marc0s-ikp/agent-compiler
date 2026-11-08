int const_index(void);
int var_index(int i);
int terminator(void);
int deref(void);
int ptr_add(void);
int addr_diff(void);

int main()
{
    if (const_index() != 'b') return 1;
    if (var_index(0) != 'x') return 2;
    if (var_index(2) != 'z') return 3;
    if (terminator() != 0) return 4;
    if (deref() != 'Q') return 5;
    if (ptr_add() != 'o') return 6;
    if (addr_diff() != 1) return 7;
    return 0;
}
