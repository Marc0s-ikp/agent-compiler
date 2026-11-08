int f(int a, int b, int c);

int main()
{
    if (!(f(100,5,2)==93*100000+10*1000+0+(100<<5<<2))) return 1;
    if (!(f(7,3,2)==2*100000+1*1000+1+(7<<3<<2))) return 2;
    return 0;
}
