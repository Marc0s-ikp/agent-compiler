int sum_mixed(int a, int b, int c, int d, int e, int f, int g, int h,
              signed char sc, unsigned short us, int *p);
int *tail_ptr(int a, int b, int c, int d, int e, int f, int g, int h, int *p, int *q);
int *pick(int *x, int *y, int sel);
int call_sum(int *p);
int call_tail(int *p);

int main(void)
{
    int v[2] = {7, 9};
    if (sum_mixed(1, 2, 3, 4, 5, 6, 7, 8, -5, 65535, v) != 36 - 500 + 65535 + 7) return 1;
    if (sum_mixed(0, 0, 0, 0, 0, 0, 0, 0, 127, 0, v + 1) != 12700 + 9) return 2;
    if (call_sum(v) != 36 - 500 + 65535 + 7) return 3;
    if (tail_ptr(1, 2, 3, 4, 5, 6, 7, 8, v, v + 1) != v) return 4;
    if (tail_ptr(2, 2, 3, 4, 5, 6, 7, 8, v, v + 1) != v + 1) return 5;
    if (pick(v, v + 1, 1) != v || pick(v, v + 1, 0) != v + 1) return 6;
    if (call_tail(v) != 14) return 7;
    return 0;
}
