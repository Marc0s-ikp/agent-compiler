int local_fill(void);
int global_fill(void);
int neighbours(void);
int sizeof_elems(void);
extern char gbuf[4];

int main()
{
    if (local_fill() != 600) return 1;
    if (global_fill() != 250 + 251 * 10 + 253) return 2;
    if (gbuf[2] != 252) return 3;
    if (gbuf[3] != 253) return 4;
    if (neighbours() != 10000 + 7700 + 3) return 5;
    if (sizeof_elems() != 7) return 6;
    return 0;
}
