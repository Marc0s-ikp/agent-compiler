int oct1(void);
int oct3(void);
int oct_hi(void);
int hex1(void);
int hex_hi(void);
int hex_lo(void);
int type_is_int(void);

int main()
{
    if (oct1() != 1) return 1;
    if (oct3() != 65) return 2;
    if (oct_hi() != 255) return 3;
    if (hex1() != 65) return 4;
    if (hex_hi() != 255) return 5;
    if (hex_lo() != 10) return 6;
    if (type_is_int() != 4) return 7;
    return 0;
}
