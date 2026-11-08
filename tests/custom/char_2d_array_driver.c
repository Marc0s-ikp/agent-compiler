int global_rows(void);
int global_pad(void);
int local_rows(void);
int local_pad_after_dirty(void);
int sizes(void);
char *row(int i);
int infer_rows(void);
int strcmp(const char *a, const char *b);

int main()
{
    int want;
    want = 97 * 1000000 + 98 * 10000 + 99 * 100 + 100;
    if (global_rows() != want) return 1;
    if (global_pad() != 0) return 2;
    if (local_rows() != want) return 3;
    if (local_pad_after_dirty() != 0) return 4;
    if (sizes() != 804) return 5;
    if (strcmp(row(1), "cd") != 0) return 6;
    if (infer_rows() != 3) return 7;
    return 0;
}
