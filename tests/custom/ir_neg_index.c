/* Negative indexes through a pointer into the middle of an array: a constant
   index, a variable index negated, a short variable, and writes. */
int read_back(int *mid, int n) {
    return mid[-1] * 100 + mid[-n] * 10 + mid[0];
}

int read_short(int *mid, short s) {
    return mid[s];
}

char read_char(char *mid, int n) {
    return mid[-n];
}

void write_back(int *mid, int n) {
    mid[-1] = 70;
    mid[-n] = 80;
}
