int glob(void);
int en(void);
int arr_len(void);
int sw(int v);
int local_arr(void);
int expr_arr(void);

int main()
{
    if (glob() != 807) return 1;
    if (en() != 43) return 2;
    if (arr_len() != 4) return 3;
    if (sw(4) != 1) return 4;
    if (sw(1) != 2) return 5;
    if (sw(9) != 3) return 6;
    if (local_arr() != 16) return 7;
    if (expr_arr() != 5) return 8;
    return 0;
}
