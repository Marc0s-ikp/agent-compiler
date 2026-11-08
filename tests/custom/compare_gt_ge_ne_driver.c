int f(int x, int y);

int main()
{
    if (!(f(3,2)==7)) return 1;
    if (!(f(2,2)==2)) return 2;
    if (!(f(1,2)==0+1)) return 3;
    if (!(f(-1,-2)==7)) return 4;
    if (!(f(-2,-1)==1)) return 5;
    return 0;
}
