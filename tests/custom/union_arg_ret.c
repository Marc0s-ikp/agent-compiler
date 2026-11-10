/* Unions are passed and returned with the INTEGER convention even when a
   member is float/double: a union containing a float (4 bytes) travels in a0,
   an 8-byte union in a0 and a1; none of them use fa registers. */
union UF { float f; int i; };
union UD { double d; int i; };
union UC { char c; int i; };

int ours_uf(union UF u, int n) { return u.i + n; }
double ours_ud(union UD u, int n) { return u.d + n; }
int ours_uc(union UC u, int n) { return u.c + n; }
union UF ours_mkuf(float v) { union UF u; u.f = v; return u; }
union UD ours_mkud(double v) { union UD u; u.d = v; return u; }
union UC ours_mkuc(int v) { union UC u; u.i = v; return u; }

int theirs_uf(union UF u, int n);
double theirs_ud(union UD u, int n);
int theirs_uc(union UC u, int n);
union UF theirs_mkuf(float v);
union UD theirs_mkud(double v);
union UC theirs_mkuc(int v);

int call_uf(void) { union UF u; u.i = 40; return theirs_uf(u, 2); }
double call_ud(void) { union UD u; u.d = 40.5; return theirs_ud(u, 2); }
int call_uc(void) { union UC u; u.i = 40; return theirs_uc(u, 2); }
float call_mkuf(void) { union UF u; u = theirs_mkuf(1.5f); return u.f; }
double call_mkud(void) { union UD u; u = theirs_mkud(1.5); return u.d; }
int call_mkuc(void) { union UC u; u = theirs_mkuc(77); return u.i; }
