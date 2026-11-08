int hidden = 100;
int tbl[3] = {10, 20, 30};
int get_hidden();
int get_zero();
int tbl_sum();

int main()
{
    if (get_hidden() != 8) return 1;
    if (get_hidden() != 9) return 2;
    if (hidden != 100) return 3;
    if (get_zero() != 0) return 4;
    if (tbl_sum() != 6) return 5;
    if (tbl[0] != 10) return 6;
    return 0;
}
