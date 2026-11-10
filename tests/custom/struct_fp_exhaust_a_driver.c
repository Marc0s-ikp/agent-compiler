struct FI { float f; int i; };
struct ID { int i; double d; };

double ours_fi_a7(int a, int b, int c, int d, int e, int f, int g, struct FI s, int tail);
double ours_fi_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct FI s, int tail);
double ours_id_a7(int a, int b, int c, int d, int e, int f, int g, struct ID s, int tail);
double ours_id_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct ID s, int tail);
double call_fi_a7(void);
double call_fi_stack(void);
double call_id_a7(void);
double call_id_stack(void);

double theirs_fi_a7(int a, int b, int c, int d, int e, int f, int g, struct FI s, int tail)
{ return a + g + s.f * 100.0 + s.i * 10.0 + tail; }
double theirs_fi_stack(int a, int b, int c, int d, int e, int f, int g, int h,
                     struct FI s, int tail)
{ return a + h + s.f * 100.0 + s.i * 10.0 + tail; }
double theirs_id_a7(int a, int b, int c, int d, int e, int f, int g, struct ID s, int tail)
{ return a + g + s.d * 100.0 + s.i * 10.0 + tail; }
double theirs_id_stack(int a, int b, int c, int d, int e, int f, int g, int h,
                     struct ID s, int tail)
{ return a + h + s.d * 100.0 + s.i * 10.0 + tail; }

int main()
{
    struct FI fi; struct ID id;
    fi.f = 1.0f; fi.i = 2; id.d = 1.0; id.i = 2;
    if (ours_fi_a7(1, 2, 3, 4, 5, 6, 7, fi, 9) != 137.0) return 1;
    if (ours_fi_stack(1, 2, 3, 4, 5, 6, 7, 8, fi, 9) != 138.0) return 2;
    if (ours_id_a7(1, 2, 3, 4, 5, 6, 7, id, 9) != 137.0) return 3;
    if (ours_id_stack(1, 2, 3, 4, 5, 6, 7, 8, id, 9) != 138.0) return 4;
    if (call_fi_a7() != 137.0) return 5;
    if (call_fi_stack() != 138.0) return 6;
    if (call_id_a7() != 137.0) return 7;
    if (call_id_stack() != 138.0) return 8;
    return 0;
}
