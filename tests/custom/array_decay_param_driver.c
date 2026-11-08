int f(int n);

int main()
{
    /* n=4: sum 0+1+4+9=14, last=9, sum(a+2,3)=4+9+16=29 */
    if (f(4) != 1400 + 9 + 29) return 1;
    if (f(10) != 28500 + 81 + 29) return 2;
    return 0;
}
