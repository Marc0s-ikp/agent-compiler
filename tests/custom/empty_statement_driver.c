int f(int x);

int main()
{
    if (!(f(7)==707)) return 1;
    if (!(f(0)==0)) return 2;
    return 0;
}
