int f(int a, int b, int c);

int main()
{
    if (!(f(3,4,5)==3+20-35+(3|(4&5)^1)+(3<<2))) return 1;
    if (!(f(1,2,3)==1+6-9+(1|(2&3)^1)+(1<<2))) return 2;
    return 0;
}
