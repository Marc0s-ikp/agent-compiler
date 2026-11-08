int f(int a, int b, int c, int d, int e, int g, int h, int i);

int main()
{
    if (f(1, 2, 3, 4, 5, 6, 7, 8) != 204) return 1;
    if (f(-1, 0, 0, 0, 0, 0, 0, 1) != 7) return 2;
    if (f(0, 0, 0, 0, 0, 0, 1, 0) != 7) return 3;
    return 0;
}
