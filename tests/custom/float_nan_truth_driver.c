int truth(double);
int nottruth(float);
int lor(double, double);
int land(float, double);
double tern(double);
int cmpne(double, double);
int cmpgt(double, double);
int cmpge(float, float);
int castchain(void);
double whiletest(double);
int uconv(void);

int main(void)
{
    volatile double zero = 0.0;
    double nan = zero / zero;
    if (!truth(nan)) return 1;
    if (truth(-0.0)) return 2;
    if (!truth(0.5)) return 3;
    if (!nottruth(0.0f) || nottruth(nan)) return 4;
    if (lor(0.0, nan) != 1 || lor(0.0, -0.0) != 0) return 5;
    if (land(1.0f, nan) != 1 || land(0.0f, 1.0) != 0) return 6;
    if (tern(nan) != 1.5 || tern(0) != 2) return 7;
    if (!cmpne(nan, nan) || cmpgt(nan, 1) || cmpgt(1, nan) || cmpge(nan, nan)) return 8;
    if (castchain() != 44) return 9;
    if (whiletest(4.0) != 10.0) return 10;
    if (!uconv()) return 11;
    return 0;
}
