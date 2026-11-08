int f(int x)
{
    int i;
    int j;
    int n = 0;
    for (i = 0; i < x; i++) {
        for (j = 0; j < 10; j++) {
            if (j == 3)
                break;
            if (j == 1)
                continue;
            n = n + 1;
        }
        if (i == 1)
            continue;
        n = n + 100;
    }
    return n;
}
