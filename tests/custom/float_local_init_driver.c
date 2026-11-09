double init_scalars(void); double init_from_call(void); float init_array(void);
double init_array_inferred(void); double init_array_expr(double x);
double init_2d(void); double init_uses_earlier(void); double static_counter(void);

int main()
{
    float c = 0.1;
    double want = 1000.0 + 100.0 + 35.0 + 2 + 3.0 - c;
    if (init_scalars() != want) return 1;
    if (init_from_call() != 7.5) return 2;
    if (init_array() != 1.0f + 25.0f + 350.0f) return 3;
    if (init_array_inferred() != 2406.5) return 4;
    if (init_array_expr(1.5) != 1.5 + 30.0 + 250.0) return 5;
    if (init_2d() != 1.0 + 20.0 + 300.0) return 6;
    if (init_uses_earlier() != 4.5) return 7;
    if (static_counter() != 1.5 || static_counter() != 2.5) return 8;
    return 0;
}
