/* Union initialisers: a brace initialiser sets the FIRST member only.
   Global and local, tagged, untagged, via typedef. */
union Num { int i; float f; };
union Num g_num = { 77 };
union Num g_zero = { 0 };
union Fl { float f; int i; };
union Fl g_fl = { 1.0f };                  /* first member is the float */
static union Num s_num = { 5 };
typedef union { char c; int i; } Cell;
Cell g_cell = { 65 };
union { int a; char b; } g_anon = { 3 };   /* untagged, no typedef */

int global_first(void) { return g_num.i; }                  /* 77 */
int global_zero(void) { return g_zero.i; }                  /* 0 */
int global_float_first(void) { return g_fl.f == 1.0f && g_fl.i == 0x3f800000; }
int static_first(void) { return s_num.i; }                  /* 5 */
int typedef_global(void) { return g_cell.c; }               /* 65 */
int anon_global(void) { return g_anon.a; }                  /* 3 */

int local_first(void) { union Num n = { 21 }; return n.i; }            /* 21 */
int local_float(void) { union Fl f = { 2.0f }; return f.i == 0x40000000; }
int local_typedef(void) { Cell c = { 66 }; return c.c + 1; }           /* 67 */
int local_tagged_untagged(void)
{
    union { int x; char y; } u = { 9 };
    return u.x;                                                        /* 9 */
}
int typedef_sizeof(void) { return sizeof(Cell); }                       /* 4 */
int typedef_array(void)
{
    Cell a[2];
    a[0].i = 1; a[1].i = 2;
    return a[0].i + a[1].i * 10;                                       /* 21 */
}
