extern int g;
extern int arr[4];
int sum();
int set();

int main()
{
    if (g != 0) return 1;
    if (arr[3] != 0) return 2;
    if (sum() != 0) return 3;
    if (set() != 12) return 4;
    if (g != 5) return 5;
    if (arr[2] != 7) return 6;
    return 0;
}
