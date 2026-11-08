/* plain char and unsigned char are unsigned; signed char is signed. */
int plain_cmp(void)
{
    char c;
    c = 255;
    return c > 0; /* 1 */
}

int uchar_val(void)
{
    unsigned char c;
    c = 255;
    return c;
}

int schar_val(void)
{
    signed char c;
    c = 255;
    return c; /* -1 */
}

int schar_cmp(void)
{
    signed char c;
    c = 200;
    return c < 0; /* 1 */
}

int uchar_wrap(void)
{
    unsigned char c;
    c = 0;
    c = c - 1;
    return c;
}
