/* When the a registers run out, the integer half of a mixed struct has no
   register. {float,int} then goes entirely to the stack (fa0 is NOT used);
   {int,double} (16 bytes) goes by reference, the pointer on the stack.
   With a7 still free the FP convention holds: float/double in fa0, int in a7. */
struct FI { float f; int i; };
struct ID { int i; double d; };

/* a0-a6 used: FI = fa0 + a7; tail on the stack */
double ours_fi_a7(int a, int b, int c, int d, int e, int f, int g, struct FI s, int tail)
{ return a + g + s.f * 100.0 + s.i * 10.0 + tail; }

/* a0-a7 used: FI is two words on the stack, tail after them */
double ours_fi_stack(int a, int b, int c, int d, int e, int f, int g, int h,
                     struct FI s, int tail)
{ return a + h + s.f * 100.0 + s.i * 10.0 + tail; }

/* a0-a6 used: ID = a7 + fa0 */
double ours_id_a7(int a, int b, int c, int d, int e, int f, int g, struct ID s, int tail)
{ return a + g + s.d * 100.0 + s.i * 10.0 + tail; }

/* a0-a7 used: ID is passed by reference, the pointer is the first stack slot */
double ours_id_stack(int a, int b, int c, int d, int e, int f, int g, int h,
                     struct ID s, int tail)
{ return a + h + s.d * 100.0 + s.i * 10.0 + tail; }

double theirs_fi_a7(int a, int b, int c, int d, int e, int f, int g, struct FI s, int tail);
double theirs_fi_stack(int a, int b, int c, int d, int e, int f, int g, int h,
                       struct FI s, int tail);
double theirs_id_a7(int a, int b, int c, int d, int e, int f, int g, struct ID s, int tail);
double theirs_id_stack(int a, int b, int c, int d, int e, int f, int g, int h,
                       struct ID s, int tail);

double call_fi_a7(void)
{ struct FI s; s.f = 1.0f; s.i = 2; return theirs_fi_a7(1, 2, 3, 4, 5, 6, 7, s, 9); }
double call_fi_stack(void)
{ struct FI s; s.f = 1.0f; s.i = 2; return theirs_fi_stack(1, 2, 3, 4, 5, 6, 7, 8, s, 9); }
double call_id_a7(void)
{ struct ID s; s.d = 1.0; s.i = 2; return theirs_id_a7(1, 2, 3, 4, 5, 6, 7, s, 9); }
double call_id_stack(void)
{ struct ID s; s.d = 1.0; s.i = 2; return theirs_id_stack(1, 2, 3, 4, 5, 6, 7, 8, s, 9); }
