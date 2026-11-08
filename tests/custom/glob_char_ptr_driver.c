int first_g(void);
int len_h(void);
char *get_g(void);
int reassign(void);
int null_check(void);
int stat_char(void);
extern char *g;
extern char *h;
int strcmp(const char *a, const char *b);

int main()
{
    if (first_g() != 'x' * 256) return 1;
    if (len_h() != 5) return 2;
    if (strcmp(get_g(), "x") != 0) return 3;
    if (strcmp(h, "hello") != 0) return 4;
    if (null_check() != 1) return 5;
    if (stat_char() != 't') return 6;
    if (reassign() != 'y') return 7;
    if (g[0] != 'y') return 8;
    return 0;
}
