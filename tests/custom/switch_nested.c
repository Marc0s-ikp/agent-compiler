int f(int a, int b)
{
    int r = 0;
    switch (a) {
    case 1:
        switch (b) {
        case 1:
            r = 11;
            break;
        case 2:
            r = 12;
        default:
            r = r + 100;
        }
        r = r + 1000;
        break;
    case 2:
        switch (b) {
        case 1:
            return 21;
        }
        r = 2;
        break;
    default:
        r = -1;
    }
    return r;
}

int triple(int a, int b, int c)
{
    switch (a) {
    case 0:
        switch (b) {
        case 0:
            switch (c) {
            case 0:
                return 1;
            default:
                return 2;
            }
        }
        return 3;
    }
    return 4;
}
