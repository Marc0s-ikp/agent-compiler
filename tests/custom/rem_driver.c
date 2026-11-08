int f(int x, int y);

int main()
{
    if (!(f(17,5)==2)) return 1;
    if (!(f(-17,5)==-2)) return 2;
    if (!(f(17,-5)==2)) return 3;
    if (!(f(-17,-5)==-2)) return 4;
    if (!(f(10,5)==0)) return 5;
    return 0;
}
