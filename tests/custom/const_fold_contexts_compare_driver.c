int get_a(), get_b(), get_c(), get_d(), get_f();
int sw(int x);
int size_lt(), size_mix(), size_ne();

int main()
{
    if (get_a() != 1) return 1;
    if (get_b() != 0) return 2;
    if (get_c() != 1) return 3;
    if (get_d() != 1) return 4;
    if (get_f() != 0) return 5;
    /* labels: 1 < 2 is 1, -1 < 0u is 0, (-1 == ~0u) + 1 is 2, (-1 < 0u) + 7 is 7 */
    if (sw(1) != 10) return 6;
    if (sw(0) != 20) return 7;
    if (sw(2) != 30) return 8;
    if (sw(7) != 40) return 12;
    if (sw(3) != -1) return 13;
    if (size_lt() != 2) return 9;
    if (size_mix() != 3) return 10;
    if (size_ne() != 5) return 11;
    return 0;
}
