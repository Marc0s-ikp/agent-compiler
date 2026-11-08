int f(int x);

int main()
{
    if (!(f(4)==1000+20)) return 1;
    if (!(f(0)==21)) return 2;
    if (!(f(30)==46500+31)) return 3;
    return 0;
}
