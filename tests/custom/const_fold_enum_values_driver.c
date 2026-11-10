int e_div(), e_shr(), e_sar(), e_mod(), e_last();

int main()
{
    if (e_div() != (int)((0u - 2) / 2)) return 11;
    if (e_shr() != (int)(~0u >> 28)) return 12;
    if (e_sar() != -16 >> 2) return 13;
    if (e_mod() != (int)((0u - 1) % 7)) return 14;
    /* E_LAST follows E_MOD, which is 3 */
    if (e_last() != 4) return 15;
    return 0;
}
