int f(int x, int y);

int main()
{
    if (!(f(64,2)==256+16)) return 1;
    if (!(f(-64,2)==-256-16)) return 2;
    if (!(f(1,0)==2)) return 3;
    return 0;
}
