int f(int x);

int count = 0;

void bump(int n)
{
    count += n;
}

void nothing(int x)
{
    count += 1000;
}

int main()
{
    if (f(4) != 4) return 1;
    if (count != 1014) return 2;
    return 0;
}
