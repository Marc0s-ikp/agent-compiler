int list_sum(void); int chain_arrow(void); int recursive(void); int global_list(void);
int two_pointers(void);

int main()
{
    if (list_sum() != 6) return 1;
    if (chain_arrow() != 42) return 2;
    if (recursive() != 5) return 3;
    if (global_list() != 151005) return 4;
    if (two_pointers() != 321) return 5;
    return 0;
}
