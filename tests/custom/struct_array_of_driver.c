int local_array(void); int global_array(void); int nested_member_array(void);
int element_copy(void); int computed_index(void); int size_and_stride(void);

int main()
{
    if (local_array() != 74) return 1;
    if (global_array() != 54) return 2;
    if (nested_member_array() != 106105) return 3;
    if (element_copy() != 3412) return 4;
    if (computed_index() != 987) return 5;
    if (size_and_stride() != 122) return 6;
    return 0;
}
