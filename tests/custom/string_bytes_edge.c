char hi[] = "\377\200\0z\"\\";
char *off = "hello" + 1;
char *mid = &"world"[2];
static char *stat_ptr = "static ptr";

int hi_bytes(void)
{
    return hi[0] * 1000 + hi[1] - hi[2] + hi[3];
}

int hi_size(void)
{
    return sizeof(hi);
}

int quote_slash(void)
{
    return hi[4] * 1000 + hi[5];
}

int offsets(void)
{
    return off[0] * 1000 + mid[0];
}

int static_ptr(void)
{
    return stat_ptr[0];
}

int static_local_str(void)
{
    static char s[] = "abc";
    static char *p = "xyz";
    return s[1] * 1000 + p[2] + sizeof(s);
}

int octal_then_digit(void)
{
    return "\0011"[1];
}

int hex_then_end(void)
{
    return "\x41" "B"[0];
}
