int walk(char *p);
int sum_bytes(char *p, int n);
void fill(char *p, int n, int v);
int diff(char *a, char *b);
int byte_of_int2(int i);
int ptr_to_local(void);

int main()
{
    char buf[6];
    int x;
    if (walk("hello") != 5) return 1;
    if (walk("") != 0) return 2;
    buf[0] = 1; buf[1] = 2; buf[2] = 3; buf[3] = 200; buf[4] = 0; buf[5] = 9;
    if (sum_bytes(buf, 4) != 206) return 3;
    fill(buf, 6, 7);
    if (buf[0] != 7 || buf[5] != 7) return 4;
    if (diff(buf + 1, buf + 5) != 4) return 5;
    if (byte_of_int2(0) != 1) return 6;
    if (byte_of_int2(3) != 4) return 7;
    if (ptr_to_local() != 65) return 8;
    x = 0;
    return x;
}
