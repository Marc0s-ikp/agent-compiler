/* Case values are converted to the promoted type of the controlling expression. */
int sw_unsigned(unsigned u) {
    switch (u) {
        case -1: return 1;           /* becomes 0xffffffff */
        case 0x80000000u: return 2;
        case 3: return 3;
        default: return 0;
    }
}

int sw_char(unsigned char c) {
    switch (c) {                     /* promoted to int: 255 is not -1 */
        case -1: return 1;
        case 255: return 2;
        default: return 0;
    }
}

int sw_signed(int i) {
    switch (i) {
        case 0xffffffffu: return 1;  /* becomes -1 */
        case -2147483647 - 1: return 2;
        default: return 0;
    }
}
