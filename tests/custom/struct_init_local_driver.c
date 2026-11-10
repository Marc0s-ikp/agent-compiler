int full(void); int zero_rest(void); int zero_nested(void); int zero_array_member(void);
int array_of_structs(void); int elision(void); int char_array_member(void);
int doubles(void); int static_local_twice(void); int reinit(void);

int main()
{
    if (full() != 34) return 1;
    if (zero_rest() != 111) return 2;
    if (zero_nested() != 1000) return 3;
    if (zero_array_member() != 900) return 4;
    if (array_of_structs() != 41) return 5;
    if (elision() != 1242) return 6;
    if (char_array_member() != 201) return 7;
    if (doubles() != 10) return 8;
    if (static_local_twice() != 87) return 9;
    if (reinit() != 6) return 10;
    return 0;
}
