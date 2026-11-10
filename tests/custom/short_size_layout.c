/* Size, alignment, struct padding, arrays and pointer scaling. */
struct CS { char c; short s; };
struct SC { short s; char c; };
struct SI { short s; int i; };
struct SS { short a; short b; short c; };
struct CSC { char a; short b; char c; };
union U { short s; char c; };

int size_short(void) { return sizeof(short); }
int size_cs(void) { return sizeof(struct CS); }
int size_sc(void) { return sizeof(struct SC); }
int size_si(void) { return sizeof(struct SI); }
int size_ss(void) { return sizeof(struct SS); }
int size_csc(void) { return sizeof(struct CSC); }
int size_union(void) { return sizeof(union U); }
int size_array(void) { short a[5]; return sizeof a * 100 + sizeof a[0]; }
int size_expr(void) { short a = 1; return sizeof a * 100 + sizeof(a + a); }

int off_cs(void) { struct CS x; return (char *)&x.s - (char *)&x; }
int off_sc(void) { struct SC x; return (char *)&x.c - (char *)&x; }
int off_si(void) { struct SI x; return (char *)&x.i - (char *)&x; }
int off_csc(void) { struct CSC x; return ((char *)&x.c - (char *)&x) * 10 + ((char *)&x.b - (char *)&x); }
int align_ptr(void) { struct CS x; return ((int)&x.s) % 2; }

int ptr_diff(void) { short a[10]; return &a[7] - &a[2]; }
int ptr_bytes(void) { short a[10]; return (char *)&a[7] - (char *)&a[2]; }
int ptr_add(void) { short a[4]; short *p = a; return (char *)(p + 3) - (char *)a; }
int ptr_sub(void) { short a[4]; short *p = a + 3; p = p - 2; return (char *)p - (char *)a; }
int ptr_inc(void) { short a[4]; short *p = a; p++; p++; p += 1; return (char *)p - (char *)a; }
int ptr_index(void)
{
    short a[4]; short *p = a;
    a[0] = 10; a[1] = -20; a[2] = 30; a[3] = -40;
    return p[1] + *(p + 2) + *(a + 3) + 3[a];
}
int ptr_compare(void) { short a[3]; return &a[2] > &a[1] && &a[0] < &a[1] && &a[1] == a + 1; }
int two_d(void)
{
    short m[3][4]; int i, j, s = 0;
    for (i = 0; i < 3; i++) for (j = 0; j < 4; j++) m[i][j] = i * 4 + j - 6;
    for (i = 0; i < 3; i++) for (j = 0; j < 4; j++) s += m[i][j] * (i + 1);
    return s * 1000 + sizeof m + ((char *)&m[2][1] - (char *)m);
}
