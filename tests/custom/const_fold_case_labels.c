/* Wrapped/unsigned constants used as case labels. */
int sw_u(unsigned x)
{
    switch (x) {
    case (0u - 2) / 2: return 1;
    case ~0u >> 28: return 2;
    case 0u - 1: return 3;
    case 1u << 31: return 4;
    }
    return 0;
}

int sw_i(int x)
{
    switch (x) {
    case 2147483647 + 1: return 1;
    case 65536 * 65536 + 7: return 2;
    case -16 >> 2: return 3;
    case (0u - 1) % 7: return 4;
    case -7 / 2: return 5;
    }
    return 0;
}
