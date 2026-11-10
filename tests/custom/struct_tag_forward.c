/* A tag may be named before it is defined (pointers to an incomplete type,
   mutually recursive structs), and defined later. */
struct B;                                   /* forward declaration */
struct A { int x; struct B *b; };
struct B { int y; struct A *a; };

int f(struct B *p);                         /* prototype using an incomplete tag */

struct Later;
struct Later *g_ptr;

int mutual(void)
{
    struct A a;
    struct B b;
    a.x = 3; b.y = 4;
    a.b = &b; b.a = &a;
    return a.b->a->b->y * 10 + a.x;         /* 43 */
}

int f(struct B *p) { return p->y + p->a->x; }
int use_prototype(void)
{
    struct A a;
    struct B b;
    a.x = 10; b.y = 20; b.a = &a;
    return f(&b);                           /* 30 */
}

struct Later { int v; };
int define_after_pointer(void)
{
    struct Later l;
    l.v = 77;
    g_ptr = &l;
    return g_ptr->v;                        /* 77 */
}

int declare_and_define_variable(void)
{
    struct { int a; int b; } anon;          /* untagged struct, one variable */
    struct Tag { int a; } t, *pt;           /* tag + objects in one declaration */
    anon.a = 1; anon.b = 2;
    pt = &t;
    pt->a = 5;
    return anon.a + anon.b + t.a;           /* 8 */
}
