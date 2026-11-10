/* Global and static struct initialisers: flat, nested, partial (rest zero),
   with arrays and doubles, and brace elision. Uninitialised globals are zero. */
struct P { int x; int y; };
struct Mix { char c; int i; double d; };
struct Line { struct P a; struct P b; };
struct Arr { int v[3]; char tag; };

struct P g_p = { 1, 2 };
struct P g_partial = { 7 };                  /* y is 0 */
struct P g_empty;
struct Mix g_mix = { 'a', 100, 2.5 };
struct Line g_line = { { 1, 2 }, { 3, 4 } };
struct Arr g_arr = { { 5, 6, 7 }, 'z' };
struct Arr g_arr_partial = { { 8 } };        /* v[1], v[2], tag are 0 */
struct P g_pts[3] = { { 1, 2 }, { 3, 4 } }; /* third element zero */
static struct P s_p = { 40, 50 };
struct Line g_elided = { 9, 8, 7, 6 };       /* brace elision */
struct Arr g_elided_arr = { 1, 2, 3, 'q' };  /* brace elision of the array member */
struct P g_elided_pts[2] = { 1, 2, 3, 4 };

int flat(void) { return g_p.x * 10 + g_p.y; }                           /* 12 */
int partial(void) { return g_partial.x * 10 + g_partial.y; }            /* 70 */
int empty(void) { return g_empty.x + g_empty.y; }                       /* 0 */
int mix(void) { return g_mix.c + g_mix.i + (int)(g_mix.d * 2.0); }      /* 97+100+5 */
int nested(void) { return g_line.a.x * 1000 + g_line.a.y * 100 + g_line.b.x * 10 + g_line.b.y; }   /* 1234 */
int array_member(void) { return g_arr.v[0] + g_arr.v[1] * 10 + g_arr.v[2] * 100 + (g_arr.tag == 'z') * 1000; }  /* 1765 */
int array_partial(void) { return g_arr_partial.v[0] * 10 + g_arr_partial.v[1] + g_arr_partial.v[2] + g_arr_partial.tag; }  /* 80 */
int array_of_structs(void) { return g_pts[0].x + g_pts[1].y * 10 + g_pts[2].x + g_pts[2].y; }  /* 41 */
int static_p(void) { return s_p.x + s_p.y; }                            /* 90 */
int elided(void) { return g_elided.a.x * 1000 + g_elided.a.y * 100 + g_elided.b.x * 10 + g_elided.b.y; }  /* 9876 */
int elided_arr(void) { return g_elided_arr.v[0] + g_elided_arr.v[1] * 10 + g_elided_arr.v[2] * 100 + (g_elided_arr.tag == 'q') * 1000; }  /* 1321 */
int elided_pts(void) { return g_elided_pts[0].x * 1000 + g_elided_pts[0].y * 100 + g_elided_pts[1].x * 10 + g_elided_pts[1].y; }  /* 1234 */
