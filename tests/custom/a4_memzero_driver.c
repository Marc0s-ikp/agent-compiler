void dirty(void);
int sum_ints(void), sum_chars(void), sum_ptrs(void), sum_short(void), sum_odd_tail(void);

int main(void)
{
    dirty();
    if (sum_ints() != 7) return 1;
    dirty();
    if (sum_chars() != 5) return 2;
    dirty();
    if (sum_ptrs() != 0) return 3;
    dirty();
    if (sum_short() != 3) return 4;
    dirty();
    if (sum_odd_tail() != 3) return 5;
    return 0;
}
