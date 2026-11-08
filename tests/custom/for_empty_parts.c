int f(int x)
{
    int i = 0;
    int s = 0;
    for (;;) {
        s = s + i;
        i = i + 1;
        if (i > x)
            break;
    }
    for (; i < 20;)
        i = i + 5;
    return s * 100 + i;
}
