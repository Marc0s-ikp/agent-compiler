/* Floats as conditions: != 0 means true, even for 0.5 (which is 0 as int). */
int not_f(float x) { return !x; }
int not_d(double x) { return !x; }
int and_dd(double a, double b) { return a && b; }
int or_dd(double a, double b) { return a || b; }
int and_di(double a, int b) { return a && b; }
int or_fi(float a, int b) { return a || b; }
int if_d(double x) { if (x) return 1; return 0; }
int if_f(float x) { if (x) return 1; else return 2; }
int tern_d(double x) { return x ? 10 : 20; }

int while_d(double x)
{
    int n;
    n = 0;
    while (x) {
        n++;
        x = x - 0.5;
    }
    return n;
}

int for_f(void)
{
    float f;
    int n;
    n = 0;
    for (f = 2.0f; f; f = f - 0.25f) n++;
    return n;
}

int do_d(void)
{
    double d;
    int n;
    d = 1.0;
    n = 0;
    do { n++; d = d - 1.0; } while (d);
    return n;
}

/* short-circuit: the right side must not run when the left decides */
int side;
double bump(double x) { side = side + 1; return x; }

int sc_and(double a) { side = 0; return (a && bump(1.0)) + side * 10; }
int sc_or(double a) { side = 0; return (a || bump(0.0)) + side * 10; }

/* logical not of a comparison on floats, and double negation */
int not_cmp(double a, double b) { return !(a < b); }
int not_not(double a) { return !!a; }
