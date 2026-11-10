/* Member offsets, padding and sizeof for char/int/double mixes. */
struct A { char c; int i; };               /* 8 */
struct B { char c; char d; int i; };       /* 8 */
struct C { int i; char c; };               /* 8 (tail padding) */
struct D { char c; double d; };            /* 16 */
struct E { double d; char c; };            /* 16 */
struct F { char c; int i; double d; };     /* 16 */
struct G { char a; char b; char c; };      /* 3 */
struct H { float f; char c; };             /* 8 */
struct I { char c[5]; int i; };            /* 12 */
struct J { char c[3]; char d[2]; };        /* 5 */

int sz_a(void) { return sizeof(struct A); }
int sz_b(void) { return sizeof(struct B); }
int sz_c(void) { return sizeof(struct C); }
int sz_d(void) { return sizeof(struct D); }
int sz_e(void) { return sizeof(struct E); }
int sz_f(void) { return sizeof(struct F); }
int sz_g(void) { return sizeof(struct G); }
int sz_h(void) { return sizeof(struct H); }
int sz_i(void) { return sizeof(struct I); }
int sz_j(void) { return sizeof(struct J); }

int off_a_i(void) { struct A s; return (char *)&s.i - (char *)&s; }       /* 4 */
int off_b_i(void) { struct B s; return (char *)&s.i - (char *)&s; }       /* 4 */
int off_d_d(void) { struct D s; return (char *)&s.d - (char *)&s; }       /* 8 */
int off_f_i(void) { struct F s; return (char *)&s.i - (char *)&s; }       /* 4 */
int off_f_d(void) { struct F s; return (char *)&s.d - (char *)&s; }       /* 8 */
int off_i_i(void) { struct I s; return (char *)&s.i - (char *)&s; }       /* 8 */
int off_j_d(void) { struct J s; return (char *)&s.d[0] - (char *)&s; }    /* 3 */

/* members hold their own values, padding does not leak */
int values(void)
{
    struct F s;
    s.c = 1; s.i = 2; s.d = 3.5;
    return s.c * 100 + s.i * 10 + (int)(s.d * 2);      /* 127 */
}
