/* Struct assignment copies the whole object, chains right to left, and
   ternary / comma expressions can yield a struct. */
struct P { int a; int b; };
struct Big { int a; int b; int c; char tag; double d; };

int simple(void)
{
    struct P a, b;
    b.a = 3; b.b = 4;
    a.a = 0; a.b = 0;
    a = b;
    b.a = 99;                      /* a is a copy */
    return a.a * 10 + a.b;         /* 34 */
}

int chain(void)
{
    struct P a, b, c;
    c.a = 5; c.b = 6;
    a = b = c;
    c.a = 0;
    return a.a * 1000 + a.b * 100 + b.a * 10 + b.b;   /* 5656 */
}

int big_assign(void)
{
    struct Big a, b;
    b.a = 1; b.b = 2; b.c = 3; b.tag = 4; b.d = 5.5;
    a = b;
    return a.a + a.b + a.c + a.tag + (a.d == 5.5) * 100;   /* 110 */
}

int ternary(int k)
{
    struct P a, b, r;
    a.a = 1; a.b = 2; b.a = 3; b.b = 4;
    r = k ? a : b;
    return r.a * 10 + r.b;
}

int comma(void)
{
    struct P a, b, r;
    a.a = 1; a.b = 2; b.a = 7; b.b = 8;
    r = (a.a = 9, b);
    return r.a * 10 + r.b + a.a * 100;   /* 78 + 900 */
}

int assign_value(void)
{
    struct P a, b;
    b.a = 2; b.b = 3;
    return (a = b).b + a.a * 10;    /* value of assignment is the struct */
}

int self_assign_member(void)
{
    struct P a;
    a.a = 4; a.b = 5;
    a = a;
    return a.a * 10 + a.b;
}
