int f(int x);
int blocks(int x);
int stacked_labels(int x);

int main()
{
    if (f(1) != 111) return 1;
    if (f(2) != 110) return 2;
    if (f(3) != 100) return 3;
    if (f(4) != 1000) return 4;
    if (f(5) != 0) return 5;
    if (blocks(0) != 1) return 6;
    if (blocks(1) != 5) return 7;
    if (blocks(2) != 3) return 8;
    if (stacked_labels(1) != 10) return 9;
    if (stacked_labels(3) != 10) return 10;
    if (stacked_labels(5) != 20) return 11;
    if (stacked_labels(6) != 0) return 12;
    return 0;
}
