int f(int x, int y);
void swap(int *a, int *b);

int main()
{
    int a = 1, b = 2;
    /* f(5,7): swap -> x=7,y=5; x=107; y=112 */
    if (f(5, 7) != 107112) return 1;
    swap(&a, &b);
    if (a != 2 || b != 1) return 2;
    return 0;
}
