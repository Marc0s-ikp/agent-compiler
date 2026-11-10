/* Struct tags, members and ordinary identifiers live in different name
   spaces, so the same spelling can be used in each. */
struct s { int s; };                       /* tag s, member s */
struct t { int t; struct s s; };           /* member s has type struct s */
typedef int num;
struct u { num num; int x; };              /* member named like a typedef */
struct v { int v; };
int v;                                      /* global named like the tag */

int tag_member_var(void)
{
    struct s s;                             /* variable s of type struct s */
    s.s = 5;
    return s.s;
}

int nested_same_names(void)
{
    struct t t;
    t.t = 1;
    t.s.s = 2;
    return t.t * 10 + t.s.s;               /* 12 */
}

int typedef_member(void)
{
    struct u u;
    num n;
    u.num = 3;
    u.x = 4;
    n = u.num + u.x;
    return n;                               /* 7 */
}

int var_named_like_tag(void)
{
    struct v v;                             /* local v shadows global v */
    v.v = 6;
    return v.v;
}

int global_untouched(void)
{
    v = 8;
    return v;
}

int member_names_in_two_structs(void)
{
    struct s a;
    struct u b;
    a.s = 2;
    b.x = 3;
    return a.s * 10 + b.x;                  /* 23 */
}
