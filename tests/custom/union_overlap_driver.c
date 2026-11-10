int low_byte(void); int high_byte(void); int write_bytes(void); int float_bits(void);
int double_words(void); int last_write_wins(void); int addresses_equal(void);

int main()
{
    if (low_byte() != 0x78) return 1;
    if (high_byte() != 0x12) return 2;
    if (write_bytes() != 1) return 3;
    if (!float_bits()) return 4;
    if (!double_words()) return 5;
    if (!last_write_wins()) return 6;
    if (!addresses_equal()) return 7;
    return 0;
}
