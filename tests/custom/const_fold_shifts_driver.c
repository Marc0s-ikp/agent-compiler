extern unsigned g_shr_all, g_shr_28, g_ushr_neg, g_shl_u, g_shl_u_shr, g_hi_shr;
extern int g_sar_neg, g_sar_all, g_sar_count_u, g_shl_sar, g_shl_int, g_sar_min;
unsigned rt_ushr(unsigned a, int n);
int rt_sar(int a, int n);
int rt_sar_u(int a, unsigned n);
unsigned rt_ushl(unsigned a, int n);
int rt_shl(int a, int n);

/* n: folded value wrong; n+100: run-time value wrong. */
#define CHECK(n, folded, runtime, want) \
    do { if ((folded) != (want)) return n; if ((runtime) != (want)) return n + 100; } while (0)

int main()
{
    CHECK(1, g_shr_all, rt_ushr(~0u, 1), ~0u >> 1);
    CHECK(2, g_shr_28, rt_ushr(~0u, 28), ~0u >> 28);
    CHECK(3, g_ushr_neg, rt_ushr(0u - 16, 2), (0u - 16) >> 2);
    CHECK(4, g_sar_neg, rt_sar(-16, 2), -4);
    CHECK(5, g_sar_all, rt_sar(-1, 31), -1);
    CHECK(6, g_sar_count_u, rt_sar_u(-16, 2), -4);
    CHECK(7, g_shl_u, rt_ushl(1, 31), 1u << 31);
    CHECK(8, g_shl_u_shr, rt_ushr(rt_ushl(1, 31), 31), 1u);
    CHECK(9, g_hi_shr, rt_ushr(0x80000000u, 31), 1u);
    /* signed 1 << 31 is not strictly defined in C90; on RV32 it is INT_MIN */
    CHECK(10, g_shl_sar, rt_sar(rt_shl(1, 31), 31), -1);
    CHECK(11, g_shl_int, rt_shl(1, 31), -2147483647 - 1);
    CHECK(12, g_sar_min, rt_sar(-2147483647 - 1, 4), -134217728);
    return 0;
}
