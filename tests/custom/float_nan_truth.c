/* Conditions and comparisons on floats: -0.0 is false, NaN is true, and every
   ordered compare with a NaN is false. */
int truth(double d) { if (d) return 1; return 0; }
int nottruth(float f) { return !f; }
int lor(double a, double b) { return a || b; }
int land(float a, double b) { return a && b; }
double tern(double c) { return c ? 1.5 : 2; }
int cmpne(double a, double b) { return a != b; }
int cmpgt(double a, double b) { return a > b; }
int cmpge(float a, float b) { return a >= b; }
int castchain(void) { return (char)(int)300.7 + (unsigned char)-1.0 * 0; }
double whiletest(double n) { double s; s = 0; while (n) { s += n; n -= 1; } return s; }
int uconv(void) { unsigned u; u = 4000000000u; return (double)u > 3e9; }
