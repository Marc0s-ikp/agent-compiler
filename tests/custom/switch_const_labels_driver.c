int by_enum(int x);
int by_expr(int x);
int on_enum_var();

int main()
{
    if (by_enum(0) != 100) return 1;
    if (by_enum(5) != 200) return 2;
    if (by_enum(6) != 300) return 3;
    if (by_enum(7) != 0) return 4;
    if (by_expr(3) != 1) return 5;
    if (by_expr(6) != 2) return 6;
    if (by_expr(17) != 3) return 7;
    if (by_expr(-4) != 4) return 8;
    if (by_expr(15) != 5) return 9;
    if (by_expr(14) != 6) return 10;
    if (by_expr(0) != 0) return 11;
    if (on_enum_var() != 2) return 12;
    return 0;
}
