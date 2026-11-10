/* sizeof a union is its largest member, rounded up to the alignment of its
   most-aligned member. */
union A { char c; int i; };
union B { char c[5]; char d; };
union C { char c[5]; int i; };
union D { double d; char c; };
union E { char c[9]; double d; };
union F { float f; char c[3]; };

int size_a(void) { return sizeof(union A); }    /* 4 */
int size_b(void) { return sizeof(union B); }    /* 5 */
int size_c(void) { return sizeof(union C); }    /* 8 */
int size_d(void) { return sizeof(union D); }    /* 8 */
int size_e(void) { return sizeof(union E); }    /* 16 */
int size_f(void) { return sizeof(union F); }    /* 4 */
int size_var(void) { union C u; return sizeof(u); }
int size_arr(void) { union A u[3]; return sizeof(u) / sizeof(u[0]) * 100 + sizeof(u); }  /* 3*100+12 */
