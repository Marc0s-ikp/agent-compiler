extern int a;
extern int b;
extern int c;
int get();
int bump();
int get_d();

int main()
{
    if (a != 5) return 1;
    if (b != -3) return 2;
    if (c != 14) return 3;
    if (get() != 484) return 4;
    if (get_d() != 19) return 5;
    if (bump() != 6) return 6;
    if (bump() != 7) return 7;
    if (a != 7) return 8;
    return 0;
}
