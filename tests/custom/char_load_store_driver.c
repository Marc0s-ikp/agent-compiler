int store_wrap(int v);
int store_neg(void);
int global_roundtrip(int v);
int char_sum(void);

int main()
{
    if (store_wrap(65) != 65) return 1;
    if (store_wrap(200) != 200) return 2;
    if (store_wrap(300) != 44) return 3;
    if (store_wrap(256) != 0) return 4;
    if (store_neg() != 255) return 5;
    if (global_roundtrip(511) != 255) return 6;
    if (char_sum() != 300) return 7;
    return 0;
}
