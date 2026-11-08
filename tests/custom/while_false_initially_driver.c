int f(int x);

int main()
{
    if (!(f(5)==5)) return 1;
    if (!(f(103)==3100)) return 2;
    if (!(f(100)==100)) return 3;
    return 0;
}
