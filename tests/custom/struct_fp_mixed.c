/* One float member plus one integer member: the float goes in fa0 and the
   integer in a0 (FP convention), in either member order. Applies to
   {float,int}, {int,float}, {int,double}, {double,int}, {char,float}. */
struct FI { float f; int i; };
struct IF { int i; float f; };
struct ID { int i; double d; };
struct DI { double d; int i; };
struct CF { char c; float f; };

double ours_fi(struct FI s) { return s.f * 100.0 + s.i; }
double ours_if(struct IF s) { return s.f * 100.0 + s.i; }
double ours_id(struct ID s) { return s.d * 100.0 + s.i; }
double ours_di(struct DI s) { return s.d * 100.0 + s.i; }
double ours_cf(struct CF s) { return s.f * 100.0 + s.c; }
struct FI ours_mkfi(void) { struct FI s; s.f = 1.5f; s.i = 7; return s; }
struct IF ours_mkif(void) { struct IF s; s.f = 1.5f; s.i = 7; return s; }
struct ID ours_mkid(void) { struct ID s; s.d = 1.5; s.i = 7; return s; }
struct DI ours_mkdi(void) { struct DI s; s.d = 1.5; s.i = 7; return s; }
struct CF ours_mkcf(void) { struct CF s; s.f = 1.5f; s.c = 7; return s; }

double theirs_fi(struct FI s);
double theirs_if(struct IF s);
double theirs_id(struct ID s);
double theirs_di(struct DI s);
double theirs_cf(struct CF s);
struct FI theirs_mkfi(void);
struct IF theirs_mkif(void);
struct ID theirs_mkid(void);
struct DI theirs_mkdi(void);
struct CF theirs_mkcf(void);

double call_fi(void) { struct FI s; s.f = 1.5f; s.i = 7; return theirs_fi(s); }
double call_if(void) { struct IF s; s.f = 1.5f; s.i = 7; return theirs_if(s); }
double call_id(void) { struct ID s; s.d = 1.5; s.i = 7; return theirs_id(s); }
double call_di(void) { struct DI s; s.d = 1.5; s.i = 7; return theirs_di(s); }
double call_cf(void) { struct CF s; s.f = 1.5f; s.c = 7; return theirs_cf(s); }
double call_mkfi(void) { struct FI s; s = theirs_mkfi(); return s.f * 100.0 + s.i; }
double call_mkif(void) { struct IF s; s = theirs_mkif(); return s.f * 100.0 + s.i; }
double call_mkid(void) { struct ID s; s = theirs_mkid(); return s.d * 100.0 + s.i; }
double call_mkdi(void) { struct DI s; s = theirs_mkdi(); return s.d * 100.0 + s.i; }
double call_mkcf(void) { struct CF s; s = theirs_mkcf(); return s.f * 100.0 + s.c; }
