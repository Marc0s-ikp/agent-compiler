int f(int x)
{
    int i = 0;
    int a = 0;
    while (1) {
        if (i >= x)
            break;
        i++;
    }
    do {
        a = a + 1;
        if (a == 4)
            break;
    } while (1);
    return i * 10 + a;
}
