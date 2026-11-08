int f(int x);

int main()
{
    if (!(f(1)==800)) return 1;
    if (!(f(0)==7)) return 2;
    return 0;
}
