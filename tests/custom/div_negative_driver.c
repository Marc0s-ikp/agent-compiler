int f(int x, int y);

int main()
{
    if (!(f(-7,2)==-3)) return 1;
    if (!(f(7,-2)==-3)) return 2;
    if (!(f(-7,-2)==3)) return 3;
    if (!(f(1,2)==0)) return 4;
    if (!(f(-1,2)==0)) return 5;
    return 0;
}
