/* Initialisers for short arrays and members; static, typedef, enum. */
typedef short word;
typedef unsigned short uword;
enum E { LOW = -300, MID = 0, HIGH = 30000 };
struct P { short a; char b; unsigned short c; short d[2]; };

short g_arr[4] = {-1, 2, -3, 4};
short g_part[5] = {7, -8};
short g_auto[] = {-100, 200, -300};
uword g_u[3] = {65535, 40000, 1};
struct P g_p = {-5, 'x', 65000, {-6, 7}};
struct P g_ps[2] = {{-1, 'a', 1, {-2, -3}}, {-4, 'b', 2, {-5, -6}}};
word g_word = -77;
enum E g_e = LOW;
short g_from_enum = HIGH;
short g_neg_const = -32768;
short g_wrap = 70000;
short g_expr = 3 * -7 + 1;
short g_2d[2][2] = {{-1, -2}, {-3, -4}};

int sum_arr(void) { return g_arr[0] + g_arr[1] + g_arr[2] + g_arr[3]; }
int part(void) { return g_part[0] * 1000 + g_part[1] + g_part[2] + g_part[3] + g_part[4]; }
int autosize(void) { return sizeof g_auto / sizeof g_auto[0] * 1000 + g_auto[2]; }
int ushorts(void) { return g_u[0] + g_u[1] + g_u[2]; }
int gp(void) { return g_p.a * 100000 + g_p.b * 1000 + (g_p.c == 65000) * 100 + g_p.d[0] * 10 + g_p.d[1]; }
int gps(void) { return g_ps[0].a + g_ps[0].d[1] + g_ps[1].a + g_ps[1].d[0] + g_ps[1].c; }
int misc(void) { return g_word + g_e + g_from_enum + g_neg_const + g_wrap + g_expr; }
int two_d(void) { return g_2d[0][1] * 10 + g_2d[1][0]; }

int local_arr(void) { short a[4] = {-1, -2, 3, -4}; return a[0] + a[1] + a[2] + a[3]; }
int local_part(void) { short a[4] = {-9}; return a[0] * 10 + a[1] + a[2] + a[3]; }
int local_auto(void) { short a[] = {-1, -2, -3}; return sizeof a * 100 + a[1]; }
int local_struct(void)
{
    struct P p = {-5, 'y', 50000, {-6, -7}};
    return p.a + p.b + (p.c == 50000) + p.d[0] + p.d[1];
}
int local_enum(void) { short s = LOW; short t = HIGH; enum E e = MID; return s + t + e; }
int local_word(void) { word w = -1; uword u = w; return w * 100000 + u; }
int enum_in_short(void)
{
    short s[3];
    s[0] = LOW; s[1] = MID; s[2] = HIGH;
    return s[0] + s[1] + s[2] == 29700 && s[0] == LOW && s[2] == HIGH;
}

int counter(void)
{
    static short n = -2;
    static unsigned short u = 65535;
    n++; u++;
    return n * 1000 + u;
}
int static_arr(void)
{
    static short a[3] = {-1, -2, -3};
    a[0]--;
    return a[0] * 100 + a[1] * 10 + a[2];
}
