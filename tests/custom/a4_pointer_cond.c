/* A4: a pointer as a condition (if, while, ?:, !, &&, ||) with null and non-null values. */
int cond_if(int *p) { if (p) return 1; return 0; }
int cond_if_else(char *p) { if (p) return 10; else return 20; }
int cond_not(int *p) { return !p; }
int cond_ternary(int *p, int a, int b) { return p ? a : b; }
int cond_and(int *p, int *q) { return p && q; }
int cond_or(int *p, int *q) { return p || q; }

/* counts the characters of a string by testing the pointer itself */
int cond_while(char *p, char *end)
{
    int n = 0;
    while (p != end) {
        n = n + 1;
        p = p + 1;
    }
    return n;
}

/* walks until the pointer becomes null */
int cond_walk(int **list)
{
    int n = 0;
    int **p = list;
    while (*p) {
        n = n + **p;
        p = p + 1;
    }
    return n;
}

int cond_for(char *s)
{
    int n = 0;
    char *p;
    for (p = s; p && *p; p = p + 1) n = n + 1;
    return n;
}

int cond_while_ptr(int *p)
{
    int n = 0;
    while (p) {
        n = n + 1;
        if (n == 3) p = 0;
    }
    return n;
}
