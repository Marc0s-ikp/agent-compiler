int inner_hides(void); int inner_variable_shadow(void); int inner_other_type(void);
int call_param_shadow(void);

int main()
{
    if (inner_hides() != 41204) return 1;
    if (inner_variable_shadow() != 10) return 2;
    if (inner_other_type() != 10) return 3;
    if (call_param_shadow() != 7) return 4;
    return 0;
}
