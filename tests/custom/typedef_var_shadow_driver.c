extern int global_t;
int local_var_named_t();
int param_named_t(int T);
int after_shadow();
int inner_var_then_type();

int main()
{
    if (global_t != 3) return 1;
    if (local_var_named_t() != 6) return 2;
    if (param_named_t(8) != 16) return 3;
    if (after_shadow() != 7) return 4;
    if (inner_var_then_type() != 30) return 5;
    return 0;
}
