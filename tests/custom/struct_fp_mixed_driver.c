struct FI { float f; int i; };
struct IF { int i; float f; };
struct ID { int i; double d; };
struct DI { double d; int i; };
struct CF { char c; float f; };

double ours_fi(struct FI s);
double ours_if(struct IF s);
double ours_id(struct ID s);
double ours_di(struct DI s);
double ours_cf(struct CF s);
struct FI ours_mkfi(void);
struct IF ours_mkif(void);
struct ID ours_mkid(void);
struct DI ours_mkdi(void);
struct CF ours_mkcf(void);
double call_fi(void);
double call_if(void);
double call_id(void);
double call_di(void);
double call_cf(void);
double call_mkfi(void);
double call_mkif(void);
double call_mkid(void);
double call_mkdi(void);
double call_mkcf(void);

double theirs_fi(struct FI s) { return s.f * 100.0 + s.i; }
double theirs_if(struct IF s) { return s.f * 100.0 + s.i; }
double theirs_id(struct ID s) { return s.d * 100.0 + s.i; }
double theirs_di(struct DI s) { return s.d * 100.0 + s.i; }
double theirs_cf(struct CF s) { return s.f * 100.0 + s.c; }
struct FI theirs_mkfi(void) { struct FI s; s.f = 1.5f; s.i = 7; return s; }
struct IF theirs_mkif(void) { struct IF s; s.f = 1.5f; s.i = 7; return s; }
struct ID theirs_mkid(void) { struct ID s; s.d = 1.5; s.i = 7; return s; }
struct DI theirs_mkdi(void) { struct DI s; s.d = 1.5; s.i = 7; return s; }
struct CF theirs_mkcf(void) { struct CF s; s.f = 1.5f; s.c = 7; return s; }

int main()
{
    struct FI a; struct IF b; struct ID c; struct DI d; struct CF e;
    a.f = 1.5f; a.i = 7; b.f = 1.5f; b.i = 7; c.d = 1.5; c.i = 7;
    d.d = 1.5; d.i = 7; e.f = 1.5f; e.c = 7;
    if (ours_fi(a) != 157.0) return 1;
    if (ours_if(b) != 157.0) return 2;
    if (ours_id(c) != 157.0) return 3;
    if (ours_di(d) != 157.0) return 4;
    if (ours_cf(e) != 157.0) return 5;
    a = ours_mkfi(); if (a.f != 1.5f || a.i != 7) return 6;
    b = ours_mkif(); if (b.f != 1.5f || b.i != 7) return 7;
    c = ours_mkid(); if (c.d != 1.5 || c.i != 7) return 8;
    d = ours_mkdi(); if (d.d != 1.5 || d.i != 7) return 9;
    e = ours_mkcf(); if (e.f != 1.5f || e.c != 7) return 10;
    if (call_fi() != 157.0) return 11;
    if (call_if() != 157.0) return 12;
    if (call_id() != 157.0) return 13;
    if (call_di() != 157.0) return 14;
    if (call_cf() != 157.0) return 15;
    if (call_mkfi() != 157.0) return 16;
    if (call_mkif() != 157.0) return 17;
    if (call_mkid() != 157.0) return 18;
    if (call_mkdi() != 157.0) return 19;
    if (call_mkcf() != 157.0) return 20;
    return 0;
}
