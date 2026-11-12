int local_checks(void);
int global_checks(void);
int write_through(void);

int main()
{
    int want = 0x3F | (0xF << 8) | (0x1F << 16);
    if (local_checks() != want) return 1;
    if (global_checks() != want) return 2;
    if (write_through() != 0 + 1 + 4 + 9 + 16) return 3;
    return 0;
}
