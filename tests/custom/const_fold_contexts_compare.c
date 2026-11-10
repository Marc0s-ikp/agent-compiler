/* Comparisons as integer constant expressions in enum, case and array size. */
enum { A = 1 < 2, B = -1 < 0u, C = (0u - 1) > 0, D = 2147483647 + 1 < 0, F = 3 != 3 };
int get_a() { return A; }
int get_b() { return B; }
int get_c() { return C; }
int get_d() { return D; }
int get_f() { return F; }

int sw(int x)
{
    switch (x) {
    case 1 < 2: return 10;
    case -1 < 0u: return 20;
    case ((0u - 1) == 4294967295u) + 1: return 30;
    case (-1 < 0u) + 7: return 40;
    }
    return -1;
}

int size_lt() { char a[(1 < 2) + 1]; return sizeof(a); }
int size_mix() { char a[(-1 < 0u) + 3]; return sizeof(a); }
int size_ne() { char a[(0u - 1 >= 4294967295u) + 4]; return sizeof(a); }
