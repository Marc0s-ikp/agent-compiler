int f(int x)
{
    int r = 0;
    switch (x) {
    case 1:
        r = r + 1;
    case 2:
        r = r + 10;
    case 3:
        r = r + 100;
        break;
    case 4:
        r = r + 1000;
    }
    return r;
}

int blocks(int x)
{
    int r = 0;
    switch (x) {
    case 0: {
        r = 1;
        break;
    }
    case 1: {
        r = 2;
    }
    case 2:
        r = r + 3;
    }
    return r;
}

int stacked_labels(int x)
{
    switch (x) {
    case 1:
    case 2:
    case 3:
        return 10;
    case 4:
    case 5:
        return 20;
    }
    return 0;
}
