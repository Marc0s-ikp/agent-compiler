int f(int x);

int main()
{
    if (!(f(0)==0)) return 1;
    if (!(f(1)==800)) return 2;
    if (!(f(3)==2400)) return 3;
    return 0;
}
