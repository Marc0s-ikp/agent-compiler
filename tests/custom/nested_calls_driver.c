int f(int x);

int main()
{
    /* g(12,34)=154; g(g(g(4,4),0),1) = g(g(44,0),1)=g(440,1)=4401 */
    if (f(4) != 154 + 4401) return 1;
    return 0;
}
