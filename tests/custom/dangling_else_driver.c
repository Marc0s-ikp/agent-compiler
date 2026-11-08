int f(int a, int b);

int main()
{
    if (!(f(1,1)==1)) return 1;
    if (!(f(1,0)==2)) return 2;
    if (!(f(0,1)==0)) return 3;
    if (!(f(0,0)==0)) return 4;
    return 0;
}
