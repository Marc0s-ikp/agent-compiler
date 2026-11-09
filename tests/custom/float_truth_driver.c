int not_f(float x); int not_d(double x); int and_dd(double a, double b);
int or_dd(double a, double b); int and_di(double a, int b); int or_fi(float a, int b);
int if_d(double x); int if_f(float x); int tern_d(double x);
int while_d(double x); int for_f(void); int do_d(void);
int sc_and(double a); int sc_or(double a); int not_cmp(double a, double b);
int not_not(double a);

int main()
{
    volatile double zero = 0.0;
    double nan = zero / zero;
    double negzero = -zero;

    if (not_f(0.0f) != 1 || not_f(0.5f) != 0 || not_f(-1.0f) != 0) return 1;
    if (not_d(0.0) != 1 || not_d(0.25) != 0 || not_d(-0.0) != 1) return 2;
    if (not_d(negzero) != 1) return 3;
    if (and_dd(1.0, 2.0) != 1 || and_dd(0.0, 2.0) != 0 || and_dd(1.0, 0.0) != 0) return 4;
    if (and_dd(0.5, 0.5) != 1) return 5;
    if (or_dd(0.0, 0.0) != 0 || or_dd(0.0, 0.5) != 1 || or_dd(0.5, 0.0) != 1) return 6;
    if (and_di(0.5, 3) != 1 || and_di(0.5, 0) != 0 || and_di(0.0, 3) != 0) return 7;
    if (or_fi(0.0f, 0) != 0 || or_fi(0.0f, 5) != 1 || or_fi(0.5f, 0) != 1) return 8;
    if (if_d(0.5) != 1 || if_d(0.0) != 0) return 9;
    if (if_f(0.5f) != 1 || if_f(0.0f) != 2) return 10;
    if (tern_d(0.5) != 10 || tern_d(0.0) != 20) return 11;
    if (while_d(0.0) != 0 || while_d(2.0) != 4) return 12;
    if (for_f() != 8) return 13;
    if (do_d() != 1) return 14;
    if (sc_and(0.0) != 0) return 15;          /* bump not called */
    if (sc_and(1.0) != 11) return 16;         /* bump called once */
    if (sc_or(1.0) != 1) return 17;
    if (sc_or(0.0) != 10) return 18;
    if (not_cmp(1.0, 2.0) != 0 || not_cmp(2.0, 1.0) != 1) return 19;
    if (not_not(0.5) != 1 || not_not(0.0) != 0) return 20;
    if (not_d(nan) != 0) return 21;           /* NaN is non-zero */
    if (if_d(nan) != 1) return 22;
    return 0;
}
