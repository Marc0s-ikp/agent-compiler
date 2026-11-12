/* IR step 3: recursion, mutual recursion, and calls nested in argument lists
   inside a loop. */
int fib(int n)
{
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

int ack(int m, int n)
{
    if (m == 0) return n + 1;
    if (n == 0) return ack(m - 1, 1);
    return ack(m - 1, ack(m, n - 1));
}

int is_odd(int n);

int is_even(int n)
{
    if (n == 0) return 1;
    return is_odd(n - 1);
}

int is_odd(int n)
{
    if (n == 0) return 0;
    return is_even(n - 1);
}

int add3(int a, int b, int c) { return a * 100 + b * 10 + c; }
int twice(int x) { return x + x; }

/* nested calls as arguments, repeated: each call's result lives in its own slot */
int nested_loop(void)
{
    int i, total = 0;
    for (i = 0; i < 5; i++) {
        total += add3(twice(i), add3(i, twice(i), 1), twice(twice(i + 1)));
    }
    return total;
}

int depth(int n)
{
    if (n == 0) return 0;
    return 1 + depth(n - 1);
}
