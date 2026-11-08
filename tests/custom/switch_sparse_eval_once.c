int calls;

int next()
{
    calls = calls + 1;
    return calls;
}

int sparse(int x)
{
    switch (x) {
    case 1000:
        return 1;
    case -1000:
        return 2;
    case 100000:
        return 3;
    case 2047:
        return 4;
    case 2048:
        return 5;
    case -2049:
        return 6;
    case 0x7fffffff:
        return 7;
    }
    return 0;
}

int once(int x)
{
    int r = 0;
    calls = 0;
    switch (next() + x) {
    case 1:
        r = 10;
        break;
    case 2:
        r = 20;
        break;
    default:
        r = 30;
    }
    return r * 10 + calls;
}

int expr_switch(int x)
{
    switch (x * 2 + 1) {
    case 7:
        return 1;
    case 9:
        return 2;
    }
    return 0;
}
