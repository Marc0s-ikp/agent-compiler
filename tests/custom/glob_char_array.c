char s[] = "ab";
char t[6] = "hi";
char u[3] = {1, 2, 3};
char v[4] = {'a', 'b'};
static char w[] = "static";

int size_s(void)
{
    return sizeof(s);
}

int size_t_(void)
{
    return sizeof(t);
}

int t_tail(void)
{
    return t[2] + t[3] + t[4] + t[5];
}

int modify(void)
{
    s[0] = 'z';
    return s[0] * 256 + s[1];
}

int sum_u(void)
{
    return u[0] + u[1] + u[2];
}

int v_tail(void)
{
    return v[0] * 10000 + v[1] * 100 + v[2] + v[3];
}

int w_len(void)
{
    return sizeof(w);
}
