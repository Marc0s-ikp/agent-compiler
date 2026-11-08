int len_inferred(void);
int contents(void);
int writable(void);
int fresh_each_call(void);
int empty_str(void);
int exact_fit(void);

int main()
{
    if (len_inferred() != 4) return 1;
    if (contents() != 97 * 1000000 + 98 * 10000 + 99 * 100) return 2;
    if (writable() != 'X' + 'b') return 3;
    if (fresh_each_call() != 'b') return 4;
    if (fresh_each_call() != 'b') return 5;
    if (empty_str() != 10) return 6;
    if (exact_fit() != 'c') return 7;
    return 0;
}
