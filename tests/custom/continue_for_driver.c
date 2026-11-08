int f(int x);

int main()
{
    if (!(f(7)==1200+7)) return 1;
    if (!(f(0)==0)) return 2;
    if (!(f(1)==0+1)) return 3;
    return 0;
}
