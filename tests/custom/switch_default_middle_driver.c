int mid(int x);
int fall_through_default(int x);
int default_first(int x);
int default_only(int x);

int main()
{
    if (mid(1) != 10) return 1;
    if (mid(2) != 20) return 2;
    if (mid(3) != 99) return 3;
    if (mid(-5) != 99) return 4;
    if (fall_through_default(1) != 111) return 5;
    if (fall_through_default(2) != 110) return 6;
    if (fall_through_default(3) != 100) return 7;
    if (default_first(0) != 0) return 8;
    if (default_first(1) != 1) return 9;
    if (default_first(7) != -1) return 10;
    if (default_only(0) != 2) return 11;
    return 0;
}
