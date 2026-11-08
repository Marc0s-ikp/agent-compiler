char *hello(void);
char *empty(void);
int first(void);
int same_text_twice(void);
int call_with_literal(void);

int strcmp(const char *a, const char *b);

int main()
{
    if (strcmp(hello(), "hello") != 0) return 1;
    if (empty()[0] != 0) return 2;
    if (first() != 'w') return 3;
    if (same_text_twice() != 1) return 4;
    if (call_with_literal() != 'x' + 'y' + 'z') return 5;
    return 0;
}
