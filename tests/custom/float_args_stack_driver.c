double stk(int a, int b, int c, int d, int e, int f,
           double d1, double d2, double d3, double d4,
           double d5, double d6, double d7, double d8,
           double x, int y, double z);
float stkf(int a, int b, int c, int d, int e, int f, int g, int h,
           float f1, float f2, float f3, float f4,
           float f5, float f6, float f7, float f8,
           float x, float z);
double stkd2(int a, int b, int c, int d, int e, int f, int g, int h,
             double d1, double d2, double d3, double d4,
             double d5, double d6, double d7, double d8,
             float x, double y);
double stki(int a, int b, int c, int d, int e, int f, int g, int h,
            int i, int j, double k);
int stkmix(int a, int b, int c, int d, int e, int f, int g, int h,
           double x, char c1, int n);

int main()
{
    if (stk(1, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 5.0, 4, 3.0) != 550.0) return 1;
    if (stkf(0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2.0f, 1.0f) != 24.0f) return 2;
    if (stkd2(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2.0f, 4.0) != 42.0) return 3;
    if (stki(0, 0, 0, 0, 0, 0, 0, 0, 7, 8, 1.5) != 1578.0) return 4;
    if (stkmix(0, 0, 0, 0, 0, 0, 0, 100, 3.0, 20, 3) != 123) return 5;
    return 0;
}
