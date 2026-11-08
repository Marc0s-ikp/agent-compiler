int g;
int arr[4];

int sum()
{
    return g + arr[0] + arr[1] + arr[2] + arr[3];
}

int set()
{
    g = 5;
    arr[2] = 7;
    return sum();
}
