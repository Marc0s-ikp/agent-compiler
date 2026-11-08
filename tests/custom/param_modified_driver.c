int f(int y);

int main()
{
    if (f(5) != 210005) return 1;
    if (f(0) != 200000) return 2;
    return 0;
}
