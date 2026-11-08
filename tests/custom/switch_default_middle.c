int mid(int x)
{
    int r = 0;
    switch (x) {
    case 1:
        r = 10;
        break;
    default:
        r = 99;
        break;
    case 2:
        r = 20;
        break;
    }
    return r;
}

int fall_through_default(int x)
{
    int r = 0;
    switch (x) {
    case 1:
        r = r + 1;
    default:
        r = r + 10;
    case 3:
        r = r + 100;
    }
    return r;
}

int default_first(int x)
{
    switch (x) {
    default:
        return -1;
    case 0:
        return 0;
    case 1:
        return 1;
    }
}

int default_only(int x)
{
    int r = 1;
    switch (x) {
    default:
        r = 2;
    }
    return r;
}
