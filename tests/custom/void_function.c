void bump(int n);
void nothing(int x);

void early(int x)
{
    x = x + 1;
    return;
}

void fall_off(int x)
{
    x = x * 2;
}

int f(int x)
{
    early(x);
    fall_off(x);
    nothing(x);
    bump(x);
    bump(10);
    return x;
}
