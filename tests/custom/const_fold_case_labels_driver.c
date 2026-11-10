int sw_u(unsigned x);
int sw_i(int x);

int main()
{
    if (sw_u(2147483647u) != 1) return 1;
    if (sw_u(15) != 2) return 2;
    if (sw_u(~0u) != 3) return 3;
    if (sw_u(0x80000000u) != 4) return 4;
    if (sw_u(0) != 0) return 5;
    if (sw_i(-2147483647 - 1) != 1) return 6;
    if (sw_i(7) != 2) return 7;
    if (sw_i(-4) != 3) return 8;
    if (sw_i(3) != 4) return 9;
    if (sw_i(0) != 0) return 10;
    return 0;
}
