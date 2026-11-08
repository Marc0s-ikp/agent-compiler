int f(int x)
{
    int i = 0;
    int a = 0;
    int b = 0;
    while (i < x) {
        i++;
        if (i == 3)
            continue;
        a = a + i;
    }
    i = 0;
    do {
        i++;
        if (i == 3)
            continue;
        b = b + i;
    } while (i < x);
    return a * 1000 + b;
}
