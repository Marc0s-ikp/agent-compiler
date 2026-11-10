int flat(void); int partial(void); int empty(void); int mix(void); int nested(void);
int array_member(void); int array_partial(void); int array_of_structs(void);
int static_p(void); int elided(void); int elided_arr(void); int elided_pts(void);

int main()
{
    if (flat() != 12) return 1;
    if (partial() != 70) return 2;
    if (empty() != 0) return 3;
    if (mix() != 202) return 4;
    if (nested() != 1234) return 5;
    if (array_member() != 1765) return 6;
    if (array_partial() != 80) return 7;
    if (array_of_structs() != 41) return 8;
    if (static_p() != 90) return 9;
    if (elided() != 9876) return 10;
    if (elided_arr() != 1321) return 11;
    if (elided_pts() != 1234) return 12;
    return 0;
}
