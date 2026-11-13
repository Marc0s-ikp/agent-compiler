int sw_unsigned(unsigned u);
int sw_char(unsigned char c);
int sw_signed(int i);

int main(void) {
    if (sw_unsigned(0xffffffffu) != 1) return 1;
    if (sw_unsigned(0x80000000u) != 2) return 2;
    if (sw_unsigned(3) != 3) return 3;
    if (sw_unsigned(4) != 0) return 4;
    if (sw_char(255) != 2) return 5;
    if (sw_char(0) != 0) return 6;
    if (sw_signed(-1) != 1) return 7;
    if (sw_signed(-2147483647 - 1) != 2) return 8;
    if (sw_signed(5) != 0) return 9;
    return 0;
}
