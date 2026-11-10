int s_in(void); int s_out(void); int s_arr(void); int s_two(void); int s_deep(void);
int s_dbl(void); int s_member(void); int s_array_member(void); int s_element(void);
int s_local_array(void); int s_ptr(void); int offsets(void);

int main()
{
    if (s_in() != 8) return 1;
    if (s_out() != 12) return 2;
    if (s_arr() != 24) return 3;
    if (s_two() != 28) return 4;
    if (s_deep() != 36) return 5;
    if (s_dbl() != 24) return 6;
    if (s_member() != 841) return 7;
    if (s_array_member() != 204) return 8;
    if (s_element() != 3) return 9;
    if (s_local_array() != 80) return 10;
    if (s_ptr() != 436) return 11;
    if (offsets() != 424) return 12;
    return 0;
}
