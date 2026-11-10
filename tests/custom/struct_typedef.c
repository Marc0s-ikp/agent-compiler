/* typedef with structs: tagged, untagged, pointer typedef, array of typedef,
   typedef declared before the struct is complete, nested typedef members. */
typedef struct { int x; int y; } Point;
typedef struct Seg { Point a; Point b; } Seg;       /* typedef name = tag */
typedef struct Node Node;                            /* typedef before definition */
struct Node { int v; Node *next; };
typedef Point *PointPtr;
typedef Point Triple[3];

int point(void) { Point p; p.x = 2; p.y = 3; return p.x * 10 + p.y; }     /* 23 */
int segment(void)
{
    Seg s;
    s.a.x = 1; s.a.y = 2; s.b.x = 3; s.b.y = 4;
    return s.a.x + s.a.y * 10 + s.b.x * 100 + s.b.y * 1000;              /* 4321 */
}
int with_tag(void) { struct Seg s; s.b.y = 6; return s.b.y; }              /* 6 */
int node(void)
{
    Node a, b;
    a.v = 1; b.v = 2; a.next = &b; b.next = 0;
    return a.next->v;                                                      /* 2 */
}
int ptr_typedef(void)
{
    Point p;
    PointPtr q;
    q = &p;
    q->x = 8; q->y = 9;
    return p.x * 10 + p.y;                                                 /* 89 */
}
int array_typedef(void)
{
    Triple t;
    t[0].x = 1; t[1].x = 2; t[2].x = 3;
    return t[0].x + t[1].x * 10 + t[2].x * 100 + sizeof(Triple) * 1000;    /* 24321 */
}
Point make(int a, int b) { Point p; p.x = a; p.y = b; return p; }
int by_value(void) { Point p; p = make(6, 7); return p.x * 10 + p.y; }     /* 67 */
int sizes(void) { return sizeof(Point) * 100 + sizeof(Seg) * 10 + sizeof(PointPtr); }  /* 8*100+16*10+4 */
