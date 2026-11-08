int name_len(int i);
int count(void);
int pick(int i, int j);
extern char *names[];

int main()
{
    if (count() != 4) return 1;
    if (name_len(0) != 2) return 2;
    if (name_len(1) != 3) return 3;
    if (name_len(2) != 0) return 4;
    if (name_len(3) != 1) return 5;
    if (pick(1, 2) != 'e') return 6;
    if (names[3][0] != 'f') return 7;
    return 0;
}
