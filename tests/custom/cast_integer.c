/* Integer casts, and the cast grammar next to parenthesised expressions. */
typedef int T;
int f(int x, int *p) { return (char)x + 1 + (int)-x + (unsigned char)*p + (T)x + (x) + sizeof(int)*2 + (int)sizeof(T) + -(x) + (int)(char)(x + 300); }
char *g(void *v) { return (char *)v + 1; }
void h(int x) { (void)x; (void)(x + 1); }
