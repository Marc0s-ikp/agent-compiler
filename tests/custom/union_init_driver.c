int global_first(void); int global_zero(void); int global_float_first(void);
int static_first(void); int typedef_global(void); int anon_global(void);
int local_first(void); int local_float(void); int local_typedef(void);
int local_tagged_untagged(void); int typedef_sizeof(void); int typedef_array(void);

int main()
{
    if (global_first() != 77) return 1;
    if (global_zero() != 0) return 2;
    if (!global_float_first()) return 3;
    if (static_first() != 5) return 4;
    if (typedef_global() != 65) return 5;
    if (anon_global() != 3) return 6;
    if (local_first() != 21) return 7;
    if (!local_float()) return 8;
    if (local_typedef() != 67) return 9;
    if (local_tagged_untagged() != 9) return 10;
    if (typedef_sizeof() != 4) return 11;
    if (typedef_array() != 21) return 12;
    return 0;
}
