int f(int x);

int main()
{
    if (!(f(6)==64)) return 1;
    if (!(f(0)==4)) return 2;
    return 0;
}
