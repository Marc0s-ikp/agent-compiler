int post_inc(void);
int assign(void);
int call(void);
int div_zero(void);
int deref_null(void);
int index_oob(void);
int logical(void);
extern int counter;

int main()
{
    if (post_inc() != 405) return 1;
    if (assign() != 405) return 2;
    if (call() != 400) return 3;
    if (div_zero() != 4) return 4;
    if (deref_null() != 4) return 5;
    if (index_oob() != 4) return 6;
    counter = 0;
    if (logical() != 400) return 7;
    return 0;
}
