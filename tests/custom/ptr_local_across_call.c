void bump(int *p, int n);
int mix(int a, int b);

int f(int v)
{
    int x = v;
    int arr[3];
    arr[0] = 1; arr[1] = 2; arr[2] = 3;
    bump(&x, 10);
    bump(arr + 1, x);
    return mix(x, arr[1]) + arr[0] + arr[2];
}
