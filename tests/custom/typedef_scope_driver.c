int local_typedef_hides_variable();
int variable_visible_again();
int typedef_ends_with_block();
int same_name_two_functions_a();
int same_name_two_functions_b();

int main()
{
    if (local_typedef_hides_variable() != 4) return 1;
    if (variable_visible_again() != 3) return 2;
    if (typedef_ends_with_block() != 6) return 3;
    if (same_name_two_functions_a() != 1) return 4;
    if (same_name_two_functions_b() != 2) return 5;
    return 0;
}
