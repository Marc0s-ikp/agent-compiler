int f(int x);

int main()
{
    if (!(f(4)==4)) return 1;
    if (!(f(1)==1)) return 2;
    if (!(f(0)==1)) return 3;
    return 0;
}
