int add_wrap(void);
int inc_wrap(void);
int dec_wrap(void);
int shl_wrap(void);
int post_inc_value(void);

int main()
{
    if (add_wrap() != 4) return 1;
    if (inc_wrap() != 0) return 2;
    if (dec_wrap() != 255) return 3;
    if (shl_wrap() != 2) return 4;
    if (post_inc_value() != 255000) return 5;
    return 0;
}
