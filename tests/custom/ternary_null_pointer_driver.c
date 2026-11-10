struct S { int x; int y; };
struct N { struct N *next; int v; };
extern int null_right(struct S *p, int c), null_left(struct S *p, int c);
extern int plain_zero(struct S *p, int c), plain_zero_left(struct S *p, int c);
extern int size_int(int *ip, int c), size_struct(struct S *p, int c);
extern int size_char(char *cp, int c), size_zero(int *ip, int c);
extern int size_ptr_arith(struct S *p, int c);
extern int size_void_var(int *ip, void *v, int c), void_var_assign(int *ip, void *v, int c);
extern void *both_void(void *a, void *b, int c), *null_void(void *a, int c);
extern int *same_int(int *a, int *b, int c);
extern struct S *same_struct(struct S *a, struct S *b, int c);
extern int value_or(struct N *n, int dflt), sum_list(struct N *n);
extern struct N *next_or_null(struct N *n);
extern int second_value(struct N *head, int dflt), third_value(struct N *head, int dflt);

int main(void)
{
    struct S s;
    struct N n3, n2, n1;
    int i, j;
    char ch;
    s.x = 11; s.y = 22;
    if (null_right(&s, 1) != 11) return 1;
    if (null_left(&s, 0) != 22) return 2;
    if (plain_zero(&s, 1) != 11) return 3;
    if (plain_zero_left(&s, 0) != 22) return 4;
    if (size_int(&i, 0) != 4 || size_int(&i, 1) != 4) return 5;
    if (size_struct(&s, 0) != 8 || size_struct(&s, 1) != 8) return 6;
    if (size_char(&ch, 1) != 1) return 7;
    if (size_zero(&i, 1) != 4) return 8;
    if (size_ptr_arith(&s, 1) != 8) return 9;
    if (size_void_var(&i, &j, 0) != 4) return 10;
    if (!void_var_assign(&i, &i, 1) || !void_var_assign(&i, &i, 0)) return 11;
    if (both_void(&i, &j, 1) != (void *)&i || both_void(&i, &j, 0) != (void *)&j) return 12;
    if (same_int(&i, &j, 1) != &i || same_int(&i, &j, 0) != &j) return 13;
    if (same_struct(&s, 0, 1) != &s || same_struct(&s, 0, 0) != 0) return 14;
    if (null_void(&i, 1) != (void *)&i || null_void(&i, 0) != 0) return 15;
    n3.next = 0; n3.v = 30;
    n2.next = &n3; n2.v = 20;
    n1.next = &n2; n1.v = 10;
    if (value_or(&n1, -1) != 10 || value_or(0, -1) != -1) return 16;
    if (next_or_null(&n1) != &n2 || next_or_null(0) != 0) return 17;
    if (sum_list(&n1) != 60 || sum_list(0) != 0) return 18;
    if (second_value(&n1, -1) != 20 || second_value(&n3, -1) != -1 || second_value(0, -1) != -1) return 19;
    if (third_value(&n1, -1) != 30 || third_value(&n2, -1) != -1 || third_value(0, -1) != -1) return 20;
    return 0;
}
