struct L { struct L *n; int v; };
extern struct L a, ring[2], fixed[3], b, *pb;
extern int sz[3], *p, first, *pf;
extern struct L *local_self(void);
extern int a_loops(void), ring_walk(void), fixed_walk(void), size_of_sz(void);
extern int p_self(void), later(void), local_ok(void);

int main(void)
{
    if (a.n != &a || a.v != 3) return 1;
    if (!a_loops()) return 2;
    if (ring[0].n != &ring[1] || ring[1].n != &ring[0]) return 3;
    if (ring[0].v != 1 || ring[1].v != 2) return 4;
    if (ring_walk() != 21) return 5;
    if (fixed[0].n != &fixed[2] || fixed[1].n != &fixed[0] || fixed[2].n != &fixed[1]) return 6;
    if (fixed_walk() != 30 + 20 + 20) return 7;
    if (sz[0] != 12 || sz[1] != 5 || sz[2] != 6 || size_of_sz() != 12) return 8;
    if ((void *)p != (void *)&p || !p_self()) return 9;
    if (pf != &first || b.n != &a || pb != &b) return 10;
    if (later() != 5 + 3 + 9) return 11;
    if (!local_ok()) return 12;
    if (local_self()->n != local_self()) return 13;
    return 0;
}
