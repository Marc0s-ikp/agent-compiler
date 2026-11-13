int ptrdiff_size(void);
int ptrdiff_negative(void);
int ptrdiff_struct(void);

int main(void) {
    if (ptrdiff_size() != sizeof(char *)) return 1;
    if (ptrdiff_negative() != -7) return 2;
    if (ptrdiff_struct() != 3) return 3;
    return 0;
}
