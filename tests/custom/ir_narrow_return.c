/* Narrow return values must come back extended as the C ABI says. */
short neg_short()
{
    short x = -1;
    return x;
}

signed char neg_char()
{
    return -5;
}

unsigned char big_char()
{
    return 250;
}
