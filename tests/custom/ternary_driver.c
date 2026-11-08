int f(int x, int y);

int main()
{
    if (!(f(3,9)==91)) return 1;
    if (!(f(9,3)==91)) return 2;
    if (!(f(4,0)==42)) return 3;
    if (!(f(0,5)==53)) return 4;
    return 0;
}
