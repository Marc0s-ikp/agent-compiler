/* IR step 3: parameters in registers (all eight) and stack parameters used as
   locals, assigned to, and passed on; void functions; unprototyped calls get
   default promotions; a narrow return value is widened by its C type. */
int plain();

int eight(int a, int b, int c, int d, int e, int f, int g, int h)
{
    return a - b + c - d + e - f + g - h;
}

int assign_params(int a, char b, int c)
{
    a = a + 1;
    b = b + 1;
    c = a + b;
    return c;
}

int forward(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j)
{
    return eight(j, i, h, g, f, e, d, c) + a * 0 + b * 0;
}

short ret_short(int x) { return x; }
unsigned char ret_uchar(int x) { return x; }
signed char ret_schar(int x) { return x; }

int use_ret(void)
{
    int bad = 0;
    if (ret_short(65535) != -1) bad += 1;
    if (ret_uchar(511) != 255) bad += 2;
    if (ret_schar(255) != -1) bad += 4;
    return bad;
}

void noop(void) { }

int void_call(int x)
{
    noop();
    noop();
    return x;
}

int promoted(void)
{
    signed char c = -1;
    short s = -2;
    return plain(c, s, 'a');
}
