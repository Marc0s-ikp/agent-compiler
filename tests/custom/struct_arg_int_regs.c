/* Small integer-only structs (<= 8 bytes) are passed by value in a registers,
   one word per register. Both directions: driver (GCC) calls ours_*, and the
   call_* functions here call theirs_* compiled by GCC. */
struct W1 { int a; };
struct W2 { int a; int b; };
struct B3 { char a; char b; char c; };

int ours_w1(int x, struct W1 s, int y) { return x * 100 + s.a * 10 + y; }
int ours_w2(int x, struct W2 s, int y) { return x * 1000 + s.a * 100 + s.b * 10 + y; }
int ours_b3(struct B3 s, int y) { return s.a * 1000 + s.b * 100 + s.c * 10 + y; }

int theirs_w1(int x, struct W1 s, int y);
int theirs_w2(int x, struct W2 s, int y);
int theirs_b3(struct B3 s, int y);

int call_w1(void) { struct W1 s; s.a = 2; return theirs_w1(1, s, 3); }
int call_w2(void) { struct W2 s; s.a = 2; s.b = 3; return theirs_w2(1, s, 4); }
int call_b3(void) { struct B3 s; s.a = 1; s.b = 2; s.c = 3; return theirs_b3(s, 4); }
