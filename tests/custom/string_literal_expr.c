char *hello(void)
{
    return "hello";
}

char *empty(void)
{
    return "";
}

int first(void)
{
    char *p;
    p = "world";
    return *p;
}

int same_text_twice(void)
{
    char *a;
    char *b;
    a = "same";
    b = "same";
    return a[0] == b[0] && a[3] == b[3];
}

int pass_literal(char *s);

int call_with_literal(void)
{
    return pass_literal("xyz");
}

int pass_literal(char *s)
{
    return s[0] + s[1] + s[2];
}
