/* ?: with a pointer and a void pointer. A null pointer constant takes the
   other operand's type; a non-constant void * makes the result void *. */
struct S { int x; int y; };
struct N { struct N *next; int v; };

int null_right(struct S *p, int c) { return (c ? p : (void *)0)->x; }
int null_left(struct S *p, int c) { return (c ? (void *)0 : p)->y; }
int plain_zero(struct S *p, int c) { return (c ? p : 0)->x; }
int plain_zero_left(struct S *p, int c) { return (c ? 0 : p)->y; }

int size_int(int *ip, int c) { return sizeof *(c ? ip : (void *)0); }
int size_struct(struct S *p, int c) { return sizeof *(c ? (void *)0 : p); }
int size_char(char *cp, int c) { return sizeof *(c ? cp : (void *)0); }
int size_zero(int *ip, int c) { return sizeof *(c ? 0 : ip); }
int size_ptr_arith(struct S *p, int c) { return (int)((c ? p : (void *)0) + 1) - (int)p; }

/* non-constant void *: result is void * */
int size_void_var(int *ip, void *v, int c) { return sizeof(c ? ip : v); }
int void_var_assign(int *ip, void *v, int c)
{
    struct S *sp;
    char *cp;
    sp = c ? ip : v;
    cp = c ? v : ip;
    return (void *)sp == (void *)cp;
}

/* both void *, identical object pointers */
void *both_void(void *a, void *b, int c) { return c ? a : b; }
int *same_int(int *a, int *b, int c) { return c ? a : b; }
struct S *same_struct(struct S *a, struct S *b, int c) { return c ? a : b; }
void *null_void(void *a, int c) { return c ? a : (void *)0; }

/* list walking */
int value_or(struct N *n, int dflt) { return (n ? n : 0) ? n->v : dflt; }
struct N *next_or_null(struct N *n) { return n ? n->next : (void *)0; }
int sum_list(struct N *n)
{
    int s = 0;
    while (n) {
        s += n->v;
        n = n->next ? n->next : (void *)0;
    }
    return s;
}
int second_value(struct N *head, int dflt)
{
    struct N *n = head ? head->next : (void *)0;
    return n ? n->v : dflt;
}
int third_value(struct N *head, int dflt)
{
    return ((head ? head->next : (void *)0) ? (head->next->next ? head->next->next : (void *)0) : (void *)0) ?
        head->next->next->v : dflt;
}
