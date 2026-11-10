int e_wrap();
int e_next();

int main()
{
    if (e_wrap() != 5) return 1;
    if (e_next() != 6) return 2;
    return 0;
}
