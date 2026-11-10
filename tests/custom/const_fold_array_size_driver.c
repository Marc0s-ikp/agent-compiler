int size_g(), size_shr(), size_wrap(), size_mod(), size_sar(), size_div();

int main()
{
    if (size_g() != 16) return 17;
    if (size_shr() != 16) return 18;
    if (size_wrap() != 3) return 19;
    if (size_mod() != (int)((0u - 1) % 7) + 1) return 20;
    if (size_sar() != 6) return 21;
    if (size_div() != 2) return 22;
    return 0;
}
