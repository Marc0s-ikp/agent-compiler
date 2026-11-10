/* Wrapped/unsigned constants used as enum values. */
enum {
    E_DIV = (0u - 2) / 2,
    E_SHR = ~0u >> 28,
    E_SAR = -16 >> 2,
    E_MOD = (0u - 1) % 7,
    E_LAST
};
int e_div() { return E_DIV; }
int e_shr() { return E_SHR; }
int e_sar() { return E_SAR; }
int e_mod() { return E_MOD; }
int e_last() { return E_LAST; }
