struct FI { float f; int i; };
struct IF { int i; float f; };
struct DI { double d; int i; };
struct ID { int i; double d; };

double ours_fi(struct FI s);
double ours_if_(struct IF s);
double ours_di(int n, struct DI s);
double ours_id(struct ID s, int n);
struct FI ours_mkfi(int i);
struct IF ours_mkif(int i);
struct DI ours_mkdi(int i);
struct ID ours_mkid(int i);
double call_0(void);
double call_1(void);
double call_2(void);
double call_3(void);
double call_4(void);
double call_5(void);
double call_6(void);
double call_7(void);

double theirs_fi(struct FI s)
{ return s.f * 100.0 + s.i; }

double theirs_if_(struct IF s)
{ return s.f * 100.0 + s.i; }

double theirs_di(int n, struct DI s)
{ return n * 1000.0 + s.d * 100.0 + s.i; }

double theirs_id(struct ID s, int n)
{ return n * 1000.0 + s.d * 100.0 + s.i; }

struct FI theirs_mkfi(int i)
{ struct FI s; s.f = 1.5f; s.i = i; return s; }

struct IF theirs_mkif(int i)
{ struct IF s; s.f = 1.5f; s.i = i; return s; }

struct DI theirs_mkdi(int i)
{ struct DI s; s.d = 1.5; s.i = i; return s; }

struct ID theirs_mkid(int i)
{ struct ID s; s.d = 1.5; s.i = i; return s; }

int main(void)
{
    { struct FI s; s.f = 1.5f; s.i = 7; if ((ours_fi(s)) != (157)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (157)) return 2;   /* our caller, GCC callee */
    { struct IF s; s.f = 1.5f; s.i = 7; if ((ours_if_(s)) != (157)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (157)) return 4;   /* our caller, GCC callee */
    { struct DI s; s.d = 1.5; s.i = 7; if ((ours_di(2, s)) != (2157)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (2157)) return 6;   /* our caller, GCC callee */
    { struct ID s; s.d = 1.5; s.i = 7; if ((ours_id(s, 2)) != (2157)) return 7; }   /* GCC caller, our callee */
    if (call_3() != (2157)) return 8;   /* our caller, GCC callee */
    { struct FI v; if (((v = ours_mkfi(9), v.f * 100.0 + v.i)) != (159)) return 9; }   /* GCC caller, our callee */
    if (call_4() != (159)) return 10;   /* our caller, GCC callee */
    { struct IF v; if (((v = ours_mkif(9), v.f * 100.0 + v.i)) != (159)) return 11; }   /* GCC caller, our callee */
    if (call_5() != (159)) return 12;   /* our caller, GCC callee */
    { struct DI v; if (((v = ours_mkdi(9), v.d * 100.0 + v.i)) != (159)) return 13; }   /* GCC caller, our callee */
    if (call_6() != (159)) return 14;   /* our caller, GCC callee */
    { struct ID v; if (((v = ours_mkid(9), v.d * 100.0 + v.i)) != (159)) return 15; }   /* GCC caller, our callee */
    if (call_7() != (159)) return 16;   /* our caller, GCC callee */
    return 0;
}
