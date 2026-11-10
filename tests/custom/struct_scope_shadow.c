/* A tag declared in an inner scope hides the outer tag of the same name;
   after the block, the outer one is visible again. */
struct T { int a; };                         /* outer: 4 bytes */

int inner_hides(void)
{
    int r;
    r = sizeof(struct T);                    /* 4 */
    {
        struct T { int a; int b; int c; };   /* inner: 12 bytes */
        struct T x;
        x.a = 1; x.b = 2; x.c = 3;
        r = r * 100 + sizeof(struct T) + x.a + x.b + x.c - 6;   /* 400+12 */
    }
    return r * 100 + sizeof(struct T);      /* outer again: 41200+4 */
}

int inner_variable_shadow(void)
{
    struct T t;
    t.a = 10;
    {
        struct T t;                          /* variable shadow, same type */
        t.a = 20;
    }
    return t.a;                              /* 10 */
}

int inner_other_type(void)
{
    struct T t;
    t.a = 5;
    {
        struct T { double d; };
        struct T u;
        u.d = 2.5;
        t.a += (int)(u.d * 2.0);
    }
    return t.a;                              /* 10 */
}

int param_shadow(struct T t)
{
    {
        struct T { char c; };
        struct T u;
        u.c = 3;
        t.a += u.c;
    }
    return t.a;
}
int call_param_shadow(void) { struct T t; t.a = 4; return param_shadow(t); }   /* 7 */
