int f(int a, int b);
int triple(int a, int b, int c);

int main()
{
    if (f(1, 1) != 1011) return 1;
    if (f(1, 2) != 1112) return 2;
    if (f(1, 3) != 1100) return 3;
    if (f(2, 1) != 21) return 4;
    if (f(2, 2) != 2) return 5;
    if (f(3, 1) != -1) return 6;
    if (triple(0, 0, 0) != 1) return 7;
    if (triple(0, 0, 5) != 2) return 8;
    if (triple(0, 1, 0) != 3) return 9;
    if (triple(1, 0, 0) != 4) return 10;
    return 0;
}
