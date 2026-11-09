/* Floating args: fa0-fa7 first, then the 9th and later spill into a0.. */
float sumf10(float a, float b, float c, float d, float e,
             float f, float g, float h, float i, float j)
{
    return ((((((((a * 2.0f + b) * 2.0f + c) * 2.0f + d) * 2.0f + e) * 2.0f
             + f) * 2.0f + g) * 2.0f + h) * 2.0f + i) * 2.0f + j;
}

double sumd10(double a, double b, double c, double d, double e,
              double f, double g, double h, double i, double j)
{
    return ((((((((a * 2.0 + b) * 2.0 + c) * 2.0 + d) * 2.0 + e) * 2.0
             + f) * 2.0 + g) * 2.0 + h) * 2.0 + i) * 2.0 + j;
}

/* ints use a0.. and floats use fa0.. independently */
int mix_i(int a, double b, int c, float d, int e, double f)
{
    return a * 100 + c * 10 + e;
}

double mix_d(int a, double b, int c, float d, int e, double f)
{
    return b * 100.0 + d * 10.0 + f;
}

/* ninth double goes in a0:a1, an int after it is in a0.. of its own pool */
double nine_d_int(double a, double b, double c, double d, double e,
                  double f, double g, double h, double i, int k, int m)
{
    return i * 100.0 + k * 10 + m;
}

/* ninth float goes in a0, the int after it in a1 */
float nine_f_int(float a, float b, float c, float d, float e,
                 float f, float g, float h, float i, int k)
{
    return i * 10.0f + k;
}
