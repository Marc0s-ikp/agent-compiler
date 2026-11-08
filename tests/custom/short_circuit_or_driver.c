int f(int x);

int main()
{
    if (!(f(1)==100)) return 1;
    if (!(f(0)==105)) return 2;
    return 0;
}
