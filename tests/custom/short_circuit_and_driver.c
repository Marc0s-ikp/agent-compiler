int f(int x);

int main()
{
    if (!(f(0)==0)) return 1;
    if (!(f(1)==105)) return 2;
    return 0;
}
