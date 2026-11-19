/* A6: a record result in memory (aarch64: 24 bytes, through x8) with all eight integer argument
   registers used by arguments: x8 is not an argument register, so nothing moves. Also a 24-byte record
   argument after eight integers (by reference, the pointer on the stack). Ours calls ours and the GCC
   functions of the driver; the driver calls ours. */
struct Big { int a[6]; };

struct Big make_big(int a, int b, int c, int d, int e, int f, int g, int h)
{
    struct Big r;
    r.a[0] = a; r.a[1] = b + c; r.a[2] = d; r.a[3] = e + f; r.a[4] = g; r.a[5] = h * 2;
    return r;
}

int sum_big(int a, int b, int c, int d, int e, int f, int g, int h, struct Big s)
{
    return a + b + c + d + e + f + g + h + 100 * s.a[0] + 200 * s.a[1] + 300 * s.a[5];
}

struct Big gcc_make_big(int a, int b, int c, int d, int e, int f, int g, int h);
int gcc_sum_big(int a, int b, int c, int d, int e, int f, int g, int h, struct Big s);

int ours_to_ours(void)
{
    struct Big r;
    r = make_big(1, 2, 3, 4, 5, 6, 7, 8);
    return sum_big(1, 1, 1, 1, 1, 1, 1, 1, r) + r.a[1] + r.a[3] * 10;
}

int ours_to_gcc(void)
{
    struct Big r;
    r = gcc_make_big(1, 2, 3, 4, 5, 6, 7, 8);
    return gcc_sum_big(1, 1, 1, 1, 1, 1, 1, 1, r) + r.a[1] + r.a[3] * 10;
}
