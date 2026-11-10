/* sizeof for nested structs, array members and arrays of structs; sizeof
   applied to a type, a variable, a member and an element. */
struct In { char c; int i; };                  /* 8 */
struct Out { char c; struct In in; };          /* 12 */
struct Arr { int v[5]; char tag; };            /* 24 */
struct Two { struct In a[3]; int n; };         /* 28 */
struct Deep { struct Out o; struct Arr a; };   /* 36 */
struct Dbl { char c; struct { char x; double y; } inner; };   /* 24 */

int s_in(void) { return sizeof(struct In); }
int s_out(void) { return sizeof(struct Out); }
int s_arr(void) { return sizeof(struct Arr); }
int s_two(void) { return sizeof(struct Two); }
int s_deep(void) { return sizeof(struct Deep); }
int s_dbl(void) { return sizeof(struct Dbl); }
int s_member(void) { struct Out o; return sizeof(o.in) * 100 + sizeof(o.in.i) * 10 + sizeof(o.c); }  /* 841 */
int s_array_member(void) { struct Arr a; return sizeof(a.v) * 10 + sizeof(a.v[0]); }               /* 204 */
int s_element(void) { struct Two t; return sizeof(t.a) / sizeof(t.a[0]); }                         /* 3 */
int s_local_array(void) { struct In a[10]; return sizeof(a); }                                     /* 80 */
int s_ptr(void) { struct Deep *p; return sizeof(p) * 100 + sizeof(*p); }                           /* 436 */
int offsets(void)
{
    struct Out o;
    struct Two t;
    return ((char *)&o.in - (char *)&o) * 100 + ((char *)&t.n - (char *)&t);  /* 4*100 + 24 */
}
