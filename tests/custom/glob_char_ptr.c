char *g = "x";
char *h = "hello";
char *nothing = 0;
static char *s = "static";

int first_g(void)
{
    return g[0] * 256 + g[1];
}

int len_h(void)
{
    int n;
    n = 0;
    while (h[n]) n++;
    return n;
}

char *get_g(void)
{
    return g;
}

int reassign(void)
{
    g = "yy";
    return g[1];
}

int null_check(void)
{
    return nothing == 0;
}

int stat_char(void)
{
    return s[1];
}
