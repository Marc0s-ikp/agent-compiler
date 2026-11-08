int x = 9;
int arr[3] = {4, 5, 6};
int *p = &x;
int *q = &arr[1];
int *r = arr;
int *np = 0;
int *ptrs[2] = {&x, &arr[2]};

int combine()
{
    return *p + *q * 10 + r[2] * 100;
}

int via_p()
{
    *p = 20;
    return x;
}

int via_ptrs()
{
    return *ptrs[0] + *ptrs[1];
}

int is_null()
{
    return np == 0;
}

int same()
{
    return p == &x && r == &arr[0] && q == arr + 1;
}
