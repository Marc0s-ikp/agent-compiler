int f();
extern int a;
extern int e;

int main()
{
    if (a != 0) return 1;
    if (e != 8) return 2;
    if (f() != 348) return 3;
    return 0;
}
