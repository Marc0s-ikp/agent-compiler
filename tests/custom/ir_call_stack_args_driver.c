int mix12(int a, char b, short c, unsigned char d, unsigned short e, int f, int g,
          int h, signed char i, int j, unsigned k, short l);
int narrow_stack(int a, int b, int c, int d, int e, int f, int g, int h,
                 signed char s, unsigned char u, short t, unsigned short w);
char *first_ptr(char *p, char *q, int a, int b, int c, int d, int e, int f, int g, char *r);
int call_sum12(void);
int call_ptr(char *p, int *q);
int call_narrow(void);

int ext_sum12(int a, char b, short c, unsigned char d, unsigned short e, int f, int g,
              int h, signed char i, int j, unsigned k, short l)
{
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10 + (int)(k % 1000) + l * 11;
}

static char buf[4];
static int number;

int ext_ptr(int a, int b, int c, int d, int e, int f, int g, int h, char *p, int *q)
{
    if (p != buf || q != &number) return 1;
    return a + h;
}

char *ext_pass(char *p) { return p; }

int main()
{
    int expect;
    if (mix12(1, -2, -3, 250, 60000, 6, 7, 8, -9, 10, 4000000000u, -12)
        != ext_sum12(1, -2, -3, 250, 60000, 6, 7, 8, -9, 10, 4000000000u, -12)) return 1;
    if (mix12(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11) != ext_sum12(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)) return 2;
    if (narrow_stack(1, 2, 3, 4, 5, 6, 7, 8, -3, 200, -300, 60000) != 0) return 3;
    if (first_ptr(buf, buf + 1, 0, 0, 0, 0, 0, 0, 0, buf + 1) != buf + 1) return 4;
    if (first_ptr(buf, buf + 1, 0, 0, 0, 0, 0, 0, 0, buf) != buf) return 5;
    expect = ext_sum12(1, -2, -3, 250, 60000, 6, 7, 8, -9, 10, 4000000000u, -12);
    if (call_sum12() != expect) return 6;
    if (call_ptr(buf, &number) != 9) return 7;
    if (call_narrow() != ext_sum12(0, -1, -1, 255, 65535, 0, 0, 0, -1, 0, 0, -1)) return 8;
    return 0;
}
