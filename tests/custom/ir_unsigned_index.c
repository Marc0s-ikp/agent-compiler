/* Unsigned and signed narrow indexes: the index is widened by its own
   signedness (zext for unsigned, sext for signed), so 250 as an unsigned char
   is +250 and as a signed char is -6. */
int uc(int *a, unsigned char u) { return a[u]; }
int us(int *a, unsigned short u) { return a[u]; }
int ui(int *a, unsigned u) { return a[u]; }
int sc(int *a, signed char s) { return a[s]; }
int ss(int *a, short s) { return a[s]; }
int ch(int *a, char c) { return a[c]; }

void set_us(int *a, unsigned short u, int v) { a[u] = v; }
