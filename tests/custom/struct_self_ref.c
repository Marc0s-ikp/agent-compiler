/* Self-referential struct through a pointer: linked list built from local
   nodes, traversed, and a pointer-to-pointer walk. Also a tag used in its own
   member before the definition is complete. */
struct node { int val; struct node *next; };

int list_sum(void)
{
    struct node a, b, c;
    struct node *p;
    int s;
    a.val = 1; b.val = 2; c.val = 3;
    a.next = &b; b.next = &c; c.next = 0;
    s = 0;
    for (p = &a; p; p = p->next) s += p->val;
    return s;                                   /* 6 */
}

int chain_arrow(void)
{
    struct node a, b, c;
    a.next = &b; b.next = &c; c.val = 42;
    return a.next->next->val;                   /* 42 */
}

int length(struct node *p)
{
    if (p == 0) return 0;
    return 1 + length(p->next);
}
int recursive(void)
{
    struct node n[5];
    int i;
    for (i = 0; i < 4; i++) n[i].next = &n[i + 1];
    n[4].next = 0;
    return length(n);                           /* 5 */
}

struct node *g_head;
struct node g_nodes[3];
int global_list(void)
{
    int i, s;
    struct node *p;
    for (i = 0; i < 3; i++) { g_nodes[i].val = (i + 1) * 5; g_nodes[i].next = g_head; g_head = &g_nodes[i]; }
    s = 0;
    for (p = g_head; p != 0; p = p->next) s = s * 100 + p->val;
    return s;                                   /* 15 10 5 -> 151005 */
}

struct tree { struct tree *l; struct tree *r; int v; };
int tree_sum(struct tree *t)
{
    if (!t) return 0;
    return t->v + tree_sum(t->l) + tree_sum(t->r);
}
int two_pointers(void)
{
    struct tree a, b, c;
    a.v = 1; b.v = 20; c.v = 300;
    a.l = &b; a.r = &c;
    b.l = b.r = c.l = c.r = 0;
    return tree_sum(&a);                        /* 321 */
}
