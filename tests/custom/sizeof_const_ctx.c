int g = sizeof(int) * 2;
int h = sizeof(char[7]);
enum { N = sizeof(int), M = sizeof(char) + 2 };
int arr[sizeof(int)];

int glob(void)   { return g * 100 + h; }
int en(void)     { return N * 10 + M; }
int arr_len(void) { return sizeof(arr) / sizeof(arr[0]); }

int sw(int v)
{
    switch (v) {
    case sizeof(int): return 1;
    case sizeof(char): return 2;
    default: return 3;
    }
}

int local_arr(void)
{
    char buf[sizeof(int) * 4];
    return sizeof(buf);
}

int expr_arr(void)
{
    int x;
    char buf[sizeof(x) + 1];
    return sizeof(buf);
}
