/* p - q has type ptrdiff_t: the signed integer type of pointer width. */
int ptrdiff_size(void) {
    int a[4];
    return sizeof(&a[3] - &a[0]);
}

int ptrdiff_negative(void) {
    int a[10];
    int *p = &a[2];
    int *q = &a[9];
    return p - q;
}

int ptrdiff_struct(void) {
    struct S { int x; char c[9]; } s[5];
    return &s[4] - &s[1];
}
