int const_index(void)
{
    return "abc"[1];
}

int var_index(int i)
{
    return "xyz"[i];
}

int terminator(void)
{
    return "abc"[3];
}

int deref(void)
{
    return *"Q";
}

int ptr_add(void)
{
    return *("hello" + 4);
}

int addr_diff(void)
{
    char *s;
    s = "abcdef";
    return *(s + 2) - s[1];
}
