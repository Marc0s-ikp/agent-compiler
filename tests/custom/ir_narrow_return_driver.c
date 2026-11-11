short neg_short();
signed char neg_char();
unsigned char big_char();

int main()
{
    if (neg_short() != -1) return 1;
    if (neg_char() != -5) return 2;
    if (big_char() != 250) return 3;
    return 0;
}
