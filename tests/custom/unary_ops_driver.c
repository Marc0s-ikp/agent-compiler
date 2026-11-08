int f(int x);

int main()
{
    if (!(f(5)==-5000+250*4+0+1)) return 1;
    if (!(f(0)==0+255*4+2+1)) return 2;
    if (!(f(-3)==3000+2*4+0+1)) return 3;
    return 0;
}
