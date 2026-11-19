extern int a[4];
extern int *g_p, *g_q, *g_r;
int *rt_add(int *p, unsigned n);
int *rt_sub(int *p, unsigned n);

/* The expected pointers are the same expressions compiled by GCC: an unsigned index of 0u - 1 is
   -1 elements where unsigned and pointers are 32 bits (riscv32), +4294967295 elements on a
   64-bit target. */
int main()
{
    if (g_p != (a + 2) + (0u - 1)) return 1;
    if (rt_add(a + 2, 0u - 1) != (a + 2) + (0u - 1)) return 101;
    if (g_q != (0u - 1) + (a + 3)) return 2;
    if (rt_add(a + 3, 0u - 1) != (0u - 1) + (a + 3)) return 102;
    if (g_r != (a + 2) - 1u) return 3;
    if (rt_sub(a + 2, 1u) != (a + 2) - 1u) return 103;
    return 0;
}
