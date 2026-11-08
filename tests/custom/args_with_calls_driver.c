int f(int x);

int main()
{
    /* h(y)=10y; sum 10*(x+k)*(k+1), k=0..8 = 10*(45x + 240) */
    if (f(0) != 2400) return 1;
    if (f(1) != 2850) return 2;
    return 0;
}
