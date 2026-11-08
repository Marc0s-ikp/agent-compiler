int f(int x);

int main()
{
    if (!(f(10)==13)) return 1;
    if (!(f(3)==9)) return 2;
    return 0;
}
