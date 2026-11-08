enum Color { RED, GREEN = 5, BLUE };

int by_enum(int x)
{
    switch (x) {
    case RED:
        return 100;
    case GREEN:
        return 200;
    case BLUE:
        return 300;
    }
    return 0;
}

int by_expr(int x)
{
    switch (x) {
    case 1 + 2:
        return 1;
    case 2 * 3:
        return 2;
    case (1 << 4) | 1:
        return 3;
    case -4:
        return 4;
    case GREEN + 10:
        return 5;
    case 100 / 7:
        return 6;
    }
    return 0;
}

int on_enum_var()
{
    enum Color c;
    c = BLUE;
    switch (c) {
    case GREEN:
        return 1;
    case BLUE:
        return 2;
    }
    return 3;
}
