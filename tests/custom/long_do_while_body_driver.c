int f(int x);

int main()
{
    if (!(f(0)==800)) return 1;
    if (!(f(2)==1600)) return 2;
    return 0;
}
