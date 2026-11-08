int from_driver = 21;
int table[3] = {1, 2, 3};
int twice();
int sum_table();

int main()
{
    if (twice() != 42) return 1;
    if (sum_table() != 6) return 2;
    if (from_driver != 22) return 3;
    return 0;
}
