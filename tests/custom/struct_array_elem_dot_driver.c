int dot_on_element(void); int arrow_on_decay(void); int param_decay(void);
int star_index_equivalence(void); int array_in_struct(void);
int array_in_struct_via_ptr(void); int two_dimensional(void); int assign_through_decay(void);

int main()
{
    if (dot_on_element() != 42) return 1;
    if (arrow_on_decay() != 567) return 2;
    if (param_decay() != 37) return 3;
    if (star_index_equivalence() != 27) return 4;
    if (array_in_struct() != 134) return 5;
    if (array_in_struct_via_ptr() != 768) return 6;
    if (two_dimensional() != 36) return 7;
    if (assign_through_decay() != 8) return 8;
    return 0;
}
