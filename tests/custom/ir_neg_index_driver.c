int read_back(int *mid, int n);
int read_short(int *mid, short s);
char read_char(char *mid, int n);
void write_back(int *mid, int n);

int main()
{
    int x[10];
    char c[10];
    int k;
    for (k = 0; k < 10; k++) { x[k] = k; c[k] = 40 + k; }
    if (read_back(&x[5], 3) != 4 * 100 + 2 * 10 + 5) return 1;
    if (read_short(&x[5], -2) != 3) return 2;
    if (read_char(&c[7], 5) != 42) return 3;
    write_back(&x[5], 3);
    if (x[4] != 70 || x[2] != 80) return 4;
    if (x[3] != 3 || x[5] != 5 || x[1] != 1) return 5;
    return 0;
}
