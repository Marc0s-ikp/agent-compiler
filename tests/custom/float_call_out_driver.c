float call_f(void);
double call_d(void);
double call_mix(void);
double call_many(void);
float call_manyf(void);
double call_conv(void);
double call_nested(double x);

float ext_f(float x) { return x * 2.0f; }
double ext_d(double x) { return x + 0.5; }
double ext_mix(int a, double b, float c, int d)
{
    return a * 1000.0 + b * 100.0 + c * 10.0 + d;
}
double ext_many(double a, double b, double c, double d, double e,
                double f, double g, double h, double i, double j)
{
    return ((((((((a * 2.0 + b) * 2.0 + c) * 2.0 + d) * 2.0 + e) * 2.0
             + f) * 2.0 + g) * 2.0 + h) * 2.0 + i) * 2.0 + j;
}
float ext_manyf(float a, float b, float c, float d, float e,
                float f, float g, float h, float i, float j)
{
    return ((((((((a * 2.0f + b) * 2.0f + c) * 2.0f + d) * 2.0f + e) * 2.0f
             + f) * 2.0f + g) * 2.0f + h) * 2.0f + i) * 2.0f + j;
}

int main()
{
    if (call_f() != 4.0f) return 1;
    if (call_d() != 6.0) return 2;
    if (call_mix() != 1234.0) return 3;
    if (call_many() != 2036.0) return 4;
    if (call_manyf() != 2036.0f) return 5;
    if (call_conv() != 8.0) return 6;
    if (call_nested(1.0) != 4.75) return 7;
    return 0;
}
