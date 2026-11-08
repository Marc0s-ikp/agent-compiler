int no_default(int x)
{
    int r = 7;
    switch (x) {
    case 1:
        r = 1;
        break;
    case 2:
        r = 2;
        break;
    }
    return r;
}

int skipped_prelude(int x)
{
    int r = 0;
    switch (x) {
        r = 100;
    case 1:
        r = r + 1;
        break;
    }
    return r;
}

int empty_body(int x)
{
    int r = x;
    switch (x) {
    }
    return r;
}

int negative_cases(int x)
{
    switch (x) {
    case -1:
        return 1;
    case -2:
        return 2;
    case 0:
        return 3;
    }
    return 4;
}
