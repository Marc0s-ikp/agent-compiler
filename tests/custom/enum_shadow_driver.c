int local_hides(int x);
int param_hides(int g);
int block_enum_one();
int block_enum_two();
int block_enum_hides();
int outer_enum_after();

int main()
{
    if (local_hides(5) != 72) return 1;
    if (param_hides(9) != 9) return 2;
    if (block_enum_one() != 1) return 3;
    if (block_enum_two() != 5) return 4;
    if (block_enum_hides() != 41) return 5;
    if (outer_enum_after() != 210) return 6;
    return 0;
}
