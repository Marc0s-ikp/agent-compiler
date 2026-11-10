/* A global pointer offset by an unsigned index that wraps (+ 0xFFFFFFFF = -1). */
int a[4];
int *g_p = (a + 2) + (0u - 1);
int *g_q = (0u - 1) + (a + 3);
int *g_r = (a + 2) - 1u;

int *rt_add(int *p, unsigned n) { return p + n; }
int *rt_sub(int *p, unsigned n) { return p - n; }
