int simple(void);
int chain(void);
int big_assign(void);
int ternary(int k);
int comma(void);
int assign_value(void);
int self_assign_member(void);

int main()
{
    if (simple() != 34) return 1;
    if (chain() != 5656) return 2;
    if (big_assign() != 110) return 3;
    if (ternary(1) != 12) return 4;
    if (ternary(0) != 34) return 5;
    if (comma() != 978) return 6;
    if (assign_value() != 23) return 7;
    if (self_assign_member() != 45) return 8;
    return 0;
}
