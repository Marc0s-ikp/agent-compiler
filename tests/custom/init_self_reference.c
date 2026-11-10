/* A global's initialiser may name the object being initialised: its address,
   its elements, its size. */
struct L { struct L *n; int v; };
struct L a = {&a, 3};
struct L ring[] = {{&ring[1], 1}, {&ring[0], 2}};
struct L fixed[3] = {{&fixed[2], 10}, {&fixed[0], 20}, {&fixed[1], 30}};
int sz[3] = {sizeof sz, 5, 6};
int *p = (int *)&p;
/* a later global naming an earlier one */
int first = 5;
int *pf = &first;
struct L b = {&a, 9};
struct L *pb = &b;

struct L *local_self(void)
{
    static struct L s = {&s, 7};
    return &s;
}

int a_loops(void) { return a.n == &a && a.n->n->v == 3; }
int ring_walk(void) { return ring[0].n->v * 10 + ring[0].n->n->v; }
int fixed_walk(void) { return fixed[0].n->v + fixed[0].n->n->v + fixed[1].n->n->n->v; }
int size_of_sz(void) { return sz[0]; }
int p_self(void) { return (void *)p == (void *)&p; }
int later(void) { return *pf + pb->n->v + pb->v; }
int local_ok(void) { struct L *s = local_self(); return s->n == s && s->v == 7 && local_self() == s; }
