int f(int x)
{
    int s = 0;
    int i = 0;
    while (i < x) {
        int t = i * 2;
        int s2 = t;
        {
            int s = 100;
            s2 = s2 + s;
        }
        s = s + s2;
        i++;
    }
    return s;
}
