int f(int x);

int main()
{
    if (!(f(3)==306)) return 1;
    if (!(f(0)==0)) return 2;
    if (!(f(1)==100)) return 3;
    return 0;
}
