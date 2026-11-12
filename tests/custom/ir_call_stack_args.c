/* IR step 3: more than 8 arguments (stack arguments) mixing int, char, short,
   unsigned and pointer parameters, and our code calling a function the driver
   defines. Narrow values must arrive extended by their C signedness. */
int ext_sum12(int a, char b, short c, unsigned char d, unsigned short e, int f, int g,
              int h, signed char i, int j, unsigned k, short l);
int ext_ptr(int a, int b, int c, int d, int e, int f, int g, int h, char *p, int *q);
char *ext_pass(char *p);

/* our callee, called by the driver */
int mix12(int a, char b, short c, unsigned char d, unsigned short e, int f, int g,
          int h, signed char i, int j, unsigned k, short l)
{
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10 + (int)(k % 1000) + l * 11;
}

/* narrow values passed on the stack, then compared (sign matters) */
int narrow_stack(int a, int b, int c, int d, int e, int f, int g, int h,
                 signed char s, unsigned char u, short t, unsigned short w)
{
    int bad = 0;
    if (s != -3) bad += 1;
    if (u != 200) bad += 2;
    if (t != -300) bad += 4;
    if (w != 60000) bad += 8;
    if (a + h != 9) bad += 16;
    return bad;
}

char *first_ptr(char *p, char *q, int a, int b, int c, int d, int e, int f, int g, char *r)
{
    if (q != r) return p;
    return r;
}

/* our code calls the driver's functions */
int call_sum12(void)
{
    return ext_sum12(1, -2, -3, 250, 60000, 6, 7, 8, -9, 10, 4000000000u, -12);
}

int call_ptr(char *p, int *q)
{
    char *r = ext_pass(p);
    if (r != p) return -1;
    return ext_ptr(1, 2, 3, 4, 5, 6, 7, 8, p, q);
}

/* constants as narrow arguments are extended at compile time */
int call_narrow(void)
{
    return ext_sum12(0, 255, 65535, 255, 65535, 0, 0, 0, 255, 0, 0, 65535);
}
