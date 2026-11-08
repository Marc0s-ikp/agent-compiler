int f(int x);

int main()
{
    if (!(f(3)==40)) return 1;
    if (!(f(0)==30)) return 2;
    if (!(f(-3)==20)) return 3;
    return 0;
}
