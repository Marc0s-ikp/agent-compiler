int f(int x);

int main()
{
    if (!(f(0)==1)) return 1;
    if (!(f(-5)==1)) return 2;
    return 0;
}
