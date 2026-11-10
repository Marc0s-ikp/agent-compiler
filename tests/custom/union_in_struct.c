/* Union inside a struct, struct inside a union, and the size/offset effects. */
union Num { int i; float f; };
struct Tagged { char kind; union Num v; char tail; };     /* 1 +3 pad, 4, 1 +3 pad = 12 */

struct Pair { int a; int b; };
union Either { struct Pair p; double d; };                /* 8 */
union Wrap { struct Pair p; char c[12]; };                /* 12 */

int tagged_size(void) { return sizeof(struct Tagged); }
int tagged_offset(void)
{
    struct Tagged t;
    return (char *)&t.v - (char *)&t;                      /* 4 */
}
int tagged_use(void)
{
    struct Tagged t;
    t.kind = 1; t.tail = 9;
    t.v.i = 40;
    t.kind = 2;
    return t.v.i + t.kind + t.tail;                        /* 51 */
}
int tagged_float(void)
{
    struct Tagged t;
    t.kind = 1;
    t.v.f = 2.5f;
    return t.v.f * 2 + t.kind;                             /* 6 */
}
int either_size(void) { return sizeof(union Either); }
int either_members(void)
{
    union Either e;
    e.p.a = 3; e.p.b = 4;
    return e.p.a * 10 + e.p.b;                             /* 34 */
}
int either_overlap(void)
{
    union Either e;
    e.d = 0.0;
    e.p.b = 1;                    /* high word of the double */
    return e.d != 0.0;            /* 1 */
}
int wrap_size(void) { return sizeof(union Wrap); }
int wrap_bytes(void)
{
    union Wrap w;
    int i;
    for (i = 0; i < 12; i++) w.c[i] = i;
    return w.c[11] * 100 + w.c[4];                         /* 1104 */
}
