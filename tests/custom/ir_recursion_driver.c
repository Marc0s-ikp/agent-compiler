int fib(int n);
int ack(int m, int n);
int is_even(int n);
int is_odd(int n);
int nested_loop(void);
int depth(int n);

static int add3(int a, int b, int c) { return a * 100 + b * 10 + c; }
static int twice(int x) { return x + x; }

int main()
{
    int i, total = 0;
    if (fib(10) != 55) return 1;
    if (fib(0) != 0) return 2;
    if (ack(2, 3) != 9) return 3;
    if (ack(3, 3) != 61) return 4;
    if (!is_even(10) || is_even(7) || !is_odd(7) || is_odd(10)) return 5;
    for (i = 0; i < 5; i++) {
        total += add3(twice(i), add3(i, twice(i), 1), twice(twice(i + 1)));
    }
    if (nested_loop() != total) return 6;
    if (depth(500) != 500) return 7;
    return 0;
}
