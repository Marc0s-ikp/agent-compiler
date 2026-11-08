int f(int x);

int main()
{
    if (!(f(3)==403)) return 1;
    if (!(f(0)==100)) return 2;
    if (!(f(1)==201)) return 3;
    return 0;
}
