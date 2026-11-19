/* A6: AAPCS64 C.13. A 16-byte record when only x7 is left does not fit in two registers, so it goes on the
   stack whole and NGRN becomes 8: the integer after it is on the stack too, and x7 stays unused. Also an
   11-byte record (odd size) in x6 and x7, followed by an integer on the stack. Ours calls ours and the GCC
   functions of the driver; the driver calls ours. (On riscv32 the records are by reference or in a registers.) */
struct Q16 { int a, b, c, d; };
struct Odd11 { char c[11]; };

int one_left(int a, int b, int c, int d, int e, int f, int g, struct Q16 q, int h)
{
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 100 * q.a + 200 * q.b + 300 * q.c + 400 * q.d + 10000 * h;
}

int odd_in_regs(int a, int b, int c, int d, int e, int f, struct Odd11 o, int h)
{
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + o.c[0] * 100 + o.c[10] * 1000 + 10000 * h;
}

int gcc_one_left(int a, int b, int c, int d, int e, int f, int g, struct Q16 q, int h);
int gcc_odd_in_regs(int a, int b, int c, int d, int e, int f, struct Odd11 o, int h);

int ours_to_ours(void)
{
    struct Q16 q;
    struct Odd11 o;
    int i;
    q.a = 1; q.b = 2; q.c = 3; q.d = 4;
    for (i = 0; i < 11; i++) o.c[i] = i + 1;
    return one_left(1, 2, 3, 4, 5, 6, 7, q, 8) + odd_in_regs(1, 2, 3, 4, 5, 6, o, 9);
}

int ours_to_gcc(void)
{
    struct Q16 q;
    struct Odd11 o;
    int i;
    q.a = 1; q.b = 2; q.c = 3; q.d = 4;
    for (i = 0; i < 11; i++) o.c[i] = i + 1;
    return gcc_one_left(1, 2, 3, 4, 5, 6, 7, q, 8) + gcc_odd_in_regs(1, 2, 3, 4, 5, 6, o, 9);
}
