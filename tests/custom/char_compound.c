int add_wrap(void)
{
    char c;
    c = 250;
    c += 10;
    return c; /* 4 */
}

int inc_wrap(void)
{
    char c;
    c = 255;
    c++;
    return c; /* 0 */
}

int dec_wrap(void)
{
    char c;
    c = 0;
    --c;
    return c; /* 255 */
}

int shl_wrap(void)
{
    char c;
    c = 129;
    c <<= 1;
    return c; /* 2 */
}

int post_inc_value(void)
{
    char c;
    int old;
    c = 255;
    old = c++;
    return old * 1000 + c; /* 255000 */
}
