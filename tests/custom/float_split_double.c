/* A double that finds only a7 free: low word in a7, high word on the stack.
   Caller and callee are both compiled here (GCC's own callee misreads this
   under spike pk, so the pair is checked against each other). */
double split(int a, int b, int c, int d, int e, int f, int g, double x, int tail)
{
    double *p;
    p = &x;                      /* the callee needs an addressable copy */
    return *p * 10.0 + tail + a + g;
}

double call_split(void) { return split(1, 2, 3, 4, 5, 6, 7, 2.5, 100); }

/* the stack word after the split double is the next argument */
int split_next(int a, int b, int c, int d, int e, int f, int g, double x, int tail)
{
    return tail * 10 + g;
}

int call_split_next(void) { return split_next(1, 2, 3, 4, 5, 6, 7, 2.5, 9); }
