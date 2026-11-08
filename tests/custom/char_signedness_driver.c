int plain_cmp(void);
int uchar_val(void);
int schar_val(void);
int schar_cmp(void);
int uchar_wrap(void);

int main()
{
    if (plain_cmp() != 1) return 1;
    if (uchar_val() != 255) return 2;
    if (schar_val() != -1) return 3;
    if (schar_cmp() != 1) return 4;
    if (uchar_wrap() != 255) return 5;
    return 0;
}
