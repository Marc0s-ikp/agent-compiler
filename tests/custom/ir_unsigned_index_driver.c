int uc(int *a, unsigned char u);
int us(int *a, unsigned short u);
int ui(int *a, unsigned u);
int sc(int *a, signed char s);
int ss(int *a, short s);
int ch(int *a, char c);
void set_us(int *a, unsigned short u, int v);

static int big[70000];

int main()
{
    int k;
    int *mid;
    for (k = 0; k < 70000; k++) big[k] = k;
    mid = &big[300];
    if (uc(mid, 100) != 400) return 1;
    if (uc(mid, 250) != 550) return 2;
    if (sc(mid, 100) != 400) return 3;
    if (sc(mid, -6) != 294) return 4;
    if (ch(mid, 250) != 550) return 5;            /* plain char is unsigned */
    if (us(mid, 100) != 400) return 6;
    if (us(mid, 65535) != 65835) return 7;
    if (ss(mid, 100) != 400) return 8;
    if (ss(mid, -200) != 100) return 9;
    if (ui(mid, 3) != 303) return 10;
    if (ui(mid, 60000) != 60300) return 11;
    set_us(mid, 65535, -5);
    if (big[65835] != -5 || big[65834] != 65834) return 12;
    return 0;
}
