int run(int n)
{
    int i;
    int sum = 0;
    for (i = 0; i < n; i++) {
        switch (i % 3) {
        case 0:
            continue;
        case 1:
            sum = sum + 1;
            break;
        default:
            sum = sum + 10;
        }
        sum = sum + 100;
    }
    return sum;
}

int while_continue(int n)
{
    int i = 0;
    int hits = 0;
    while (i < n) {
        i = i + 1;
        switch (i) {
        case 2:
        case 4:
            continue;
        }
        hits = hits + 1;
    }
    return hits;
}

int break_in_inner_loop(int x)
{
    int r = 0;
    int i;
    switch (x) {
    case 1:
        for (i = 0; i < 10; i++) {
            if (i == 3) break;
            r = r + 1;
        }
        r = r + 100;
        break;
    case 2:
        r = 5;
    }
    return r;
}

int do_while_continue(int n)
{
    int i = 0;
    int s = 0;
    do {
        i = i + 1;
        switch (i & 1) {
        case 1:
            continue;
        }
        s = s + i;
    } while (i < n);
    return s;
}
