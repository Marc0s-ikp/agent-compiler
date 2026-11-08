int f(int x);

int main()
{
    if (!(f(0)==0)) return 1;
    if (!(f(2)==1600)) return 2;
    if (!(f(50)==8000)) return 3;
    return 0;
}
