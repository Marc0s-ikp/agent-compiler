int f(int x);

int main()
{
    if (!(f(3)==206)) return 1;
    if (!(f(0)==0)) return 2;
    if (!(f(2)==2*2+100)) return 3;
    return 0;
}
