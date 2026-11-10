int tag_member_var(void); int nested_same_names(void); int typedef_member(void);
int var_named_like_tag(void); int global_untouched(void); int member_names_in_two_structs(void);

int main()
{
    if (tag_member_var() != 5) return 1;
    if (nested_same_names() != 12) return 2;
    if (typedef_member() != 7) return 3;
    if (var_named_like_tag() != 6) return 4;
    if (global_untouched() != 8) return 5;
    if (member_names_in_two_structs() != 23) return 6;
    return 0;
}
