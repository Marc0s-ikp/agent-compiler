int f(int v);

void bump(int *p, int n)
{
    *p += n;
}

int mix(int a, int b)
{
    return a * 1000 + b;
}

int main()
{
    /* v=5: x=15; arr[1]=2+15=17; mix=15017; +1+3 */
    if (f(5) != 15017 + 4) return 1;
    return 0;
}
