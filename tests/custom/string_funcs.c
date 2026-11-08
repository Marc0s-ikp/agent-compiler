int my_strlen(char *s)
{
    int n;
    n = 0;
    while (s[n]) n++;
    return n;
}

void my_strcpy(char *d, char *s)
{
    while ((*d++ = *s++))
        ;
}

int my_strcmp(char *a, char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}

char *my_strchr(char *s, int c)
{
    for (; *s; s++)
        if (*s == c) return s;
    return 0;
}

int copy_and_measure(void)
{
    char buf[16];
    my_strcpy(buf, "copied!");
    return my_strlen(buf) * 100 + buf[6];
}

int literal_len(void)
{
    return my_strlen("four") + my_strlen("");
}
