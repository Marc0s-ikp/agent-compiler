extern int a[4];
extern int *g_p, *g_q, *g_r;
int *rt_add(int *p, unsigned n);
int *rt_sub(int *p, unsigned n);

int main()
{
    if (g_p != &a[1]) return 1;
    if (rt_add(a + 2, 0u - 1) != &a[1]) return 101;
    if (g_q != &a[2]) return 2;
    if (rt_add(a + 3, 0u - 1) != &a[2]) return 102;
    if (g_r != &a[1]) return 3;
    if (rt_sub(a + 2, 1u) != &a[1]) return 103;
    return 0;
}
