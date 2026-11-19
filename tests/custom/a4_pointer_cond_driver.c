int cond_if(int *p);
int cond_if_else(char *p);
int cond_not(int *p);
int cond_ternary(int *p, int a, int b);
int cond_and(int *p, int *q);
int cond_or(int *p, int *q);
int cond_while(char *p, char *end);
int cond_walk(int **list);
int cond_for(char *s);
int cond_while_ptr(int *p);

int main(void)
{
    int x = 5, y = 6;
    char s[] = "hello";
    int *list[4];
    if (cond_if(&x) != 1 || cond_if(0) != 0) return 1;
    if (cond_if_else(s) != 10 || cond_if_else(0) != 20) return 2;
    if (cond_not(&x) != 0 || cond_not(0) != 1) return 3;
    if (cond_ternary(&x, 7, 8) != 7 || cond_ternary(0, 7, 8) != 8) return 4;
    if (cond_and(&x, &y) != 1 || cond_and(&x, 0) != 0 || cond_and(0, &y) != 0 || cond_and(0, 0) != 0) return 5;
    if (cond_or(&x, &y) != 1 || cond_or(&x, 0) != 1 || cond_or(0, &y) != 1 || cond_or(0, 0) != 0) return 6;
    if (cond_while(s, s + 5) != 5 || cond_while(s, s) != 0) return 7;
    list[0] = &x; list[1] = &y; list[2] = &x; list[3] = 0;
    if (cond_walk(list) != 16) return 8;
    if (cond_for(s) != 5 || cond_for(0) != 0) return 9;
    if (cond_while_ptr(&x) != 3) return 10;
    if (cond_while_ptr(0) != 0) return 11;
    return 0;
}
