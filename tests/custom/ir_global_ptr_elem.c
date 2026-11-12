/* A global pointer initialised with the address of an element of a global
   array (&g[2], g + 3, one past the end), then read and written through it,
   including a negative index. */
int g[5] = {10, 20, 30, 40, 50};
int *gp = &g[2];
int *gq = g + 3;
int *ge = &g[5];
char cs[6] = {1, 2, 3, 4, 5, 6};
char *cp = cs + 4;

int r0(void) { return *gp; }
int r1(void) { return gp[-1]; }
int r2(void) { return *gq; }
int r3(void) { return gq[-3]; }
int r4(void) { return (int)(ge - g); }
int r5(void) { return (int)(ge - gq); }
int get(int i) { return g[i]; }
int chars(void) { return cp[-4] * 10 + cp[-1]; }

void w(void) {
    *gp = 31;
    gp[-1] = 21;
    gq[1] = 51;
    gp++;
    *gp = 41;
    gp--;
}
