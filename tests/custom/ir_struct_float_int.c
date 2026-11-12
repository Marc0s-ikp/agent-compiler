/* A struct with one floating and one integer field: the float field goes in fa0
   and the integer field in a0, in either field order, and for a double. Returns
   use fa0 and a0 the same way.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct FI { float f; int i; };
struct IF { int i; float f; };
struct DI { double d; int i; };
struct ID { int i; double d; };

double ours_fi(struct FI s)
{ return s.f * 100.0 + s.i; }

double ours_if_(struct IF s)
{ return s.f * 100.0 + s.i; }

double ours_di(int n, struct DI s)
{ return n * 1000.0 + s.d * 100.0 + s.i; }

double ours_id(struct ID s, int n)
{ return n * 1000.0 + s.d * 100.0 + s.i; }

struct FI ours_mkfi(int i)
{ struct FI s; s.f = 1.5f; s.i = i; return s; }

struct IF ours_mkif(int i)
{ struct IF s; s.f = 1.5f; s.i = i; return s; }

struct DI ours_mkdi(int i)
{ struct DI s; s.d = 1.5; s.i = i; return s; }

struct ID ours_mkid(int i)
{ struct ID s; s.d = 1.5; s.i = i; return s; }

double theirs_fi(struct FI s);
double theirs_if_(struct IF s);
double theirs_di(int n, struct DI s);
double theirs_id(struct ID s, int n);
struct FI theirs_mkfi(int i);
struct IF theirs_mkif(int i);
struct DI theirs_mkdi(int i);
struct ID theirs_mkid(int i);

double call_0(void)
{
    struct FI s; s.f = 1.5f; s.i = 7;
    return theirs_fi(s);
}

double call_1(void)
{
    struct IF s; s.f = 1.5f; s.i = 7;
    return theirs_if_(s);
}

double call_2(void)
{
    struct DI s; s.d = 1.5; s.i = 7;
    return theirs_di(2, s);
}

double call_3(void)
{
    struct ID s; s.d = 1.5; s.i = 7;
    return theirs_id(s, 2);
}

double call_4(void)
{
    struct FI v;
    return (v = theirs_mkfi(9), v.f * 100.0 + v.i);
}

double call_5(void)
{
    struct IF v;
    return (v = theirs_mkif(9), v.f * 100.0 + v.i);
}

double call_6(void)
{
    struct DI v;
    return (v = theirs_mkdi(9), v.d * 100.0 + v.i);
}

double call_7(void)
{
    struct ID v;
    return (v = theirs_mkid(9), v.d * 100.0 + v.i);
}
