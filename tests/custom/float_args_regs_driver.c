float sumf10(float a, float b, float c, float d, float e,
             float f, float g, float h, float i, float j);
double sumd10(double a, double b, double c, double d, double e,
              double f, double g, double h, double i, double j);
int mix_i(int a, double b, int c, float d, int e, double f);
double mix_d(int a, double b, int c, float d, int e, double f);
double nine_d_int(double a, double b, double c, double d, double e,
                  double f, double g, double h, double i, int k, int m);
float nine_f_int(float a, float b, float c, float d, float e,
                 float f, float g, float h, float i, int k);

int main()
{
    /* Horner base 2 over 1..10 */
    if (sumf10(1, 2, 3, 4, 5, 6, 7, 8, 9, 10) != 2036.0f) return 1;
    if (sumd10(1, 2, 3, 4, 5, 6, 7, 8, 9, 10) != 2036.0) return 2;
    if (sumd10(0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0.25) != 256.25) return 3;
    if (mix_i(1, 9.0, 2, 8.0f, 3, 7.0) != 123) return 4;
    if (mix_d(1, 9.0, 2, 8.0f, 3, 7.0) != 987.0) return 5;
    if (nine_d_int(0, 0, 0, 0, 0, 0, 0, 0, 4.0, 5, 6) != 456.0) return 6;
    if (nine_f_int(0, 0, 0, 0, 0, 0, 0, 0, 4.0f, 5) != 45.0f) return 7;
    return 0;
}
