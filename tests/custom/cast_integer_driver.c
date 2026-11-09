int f(int, int *);
char *g(void *);
void h(int);

int main(void)
{
    int v = 200;
    char buf[2];
    if (f(3, &v) != 3 + 1 + (-3) + 200 + 3 + 3 + 8 + 4 - 3 + (int)(char)303) return 1;
    if (g(buf) != buf + 1) return 2;
    h(1);
    return 0;
}
