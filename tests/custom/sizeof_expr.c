int g;
char gc;
int garr[7];

int s_int_var(void)    { int x; return sizeof(x); }
int s_char_var(void)   { char c; return sizeof(c); }
int s_noparen(void)    { int x; return sizeof x; }
int s_global(void)     { return sizeof(g) * 10 + sizeof(gc); }
int s_arr(void)        { int a[10]; return sizeof(a); }
int s_arr_elem(void)   { int a[10]; return sizeof(a[0]); }
int s_count(void)      { int a[10]; return sizeof(a) / sizeof(a[0]); }
int s_garr(void)       { return sizeof(garr) / sizeof(*garr); }
int s_deref(void)      { int *p; return sizeof(*p); }
int s_deref_ch(void)   { char *p; return sizeof(*p); }
int s_ptr(void)        { char *p; return sizeof(p); }
int s_promote(void)    { char c; return sizeof(c + 1); }  /* int */
int s_char_lit(void)   { return sizeof('a'); }            /* int in C */
int s_add(void)        { int x; return sizeof(x + 1); }
int s_cmp(void)        { int x; return sizeof(x < 1); }
int s_neg(void)        { char c; return sizeof(-c); }
int s_addr(void)       { int x; return sizeof(&x); }
int s_2d(void)         { int m[3][4]; return sizeof(m) * 100 + sizeof(m[0]) * 10 + sizeof(m[0][0]); }
int s_param(int a[10]) { return sizeof(a); }              /* decays: 4 */
int s_sizeof_sizeof(void) { return sizeof(sizeof(char)); } /* unsigned int: 4 */
int s_in_expr(void)    { int a[5]; return sizeof(a) - 1; }
