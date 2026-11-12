int eight(int a, int b, int c, int d, int e, int f, int g, int h);
int assign_params(int a, char b, int c);
int forward(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
int use_ret(void);
int void_call(int x);
int promoted(void);
short ret_short(int x);
unsigned char ret_uchar(int x);
signed char ret_schar(int x);

int plain(int a, int b, int c) { return a * 10000 + b * 100 + c; }

int main()
{
    if (eight(8, 7, 6, 5, 4, 3, 2, 1) != 4) return 1;
    if (assign_params(5, 2, 100) != 9) return 2;
    if (forward(0, 0, 1, 2, 3, 4, 5, 6, 7, 8) != eight(8, 7, 6, 5, 4, 3, 2, 1)) return 3;
    if (use_ret() != 0) return 4;
    if (void_call(42) != 42) return 5;
    if (promoted() != plain(-1, -2, 'a')) return 6;
    if (ret_short(70000) != (short)70000) return 7;
    if (ret_uchar(300) != 44) return 8;
    if (ret_schar(200) != -56) return 9;
    return 0;
}
