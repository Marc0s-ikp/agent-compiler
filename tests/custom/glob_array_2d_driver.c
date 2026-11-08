int at(int i, int j);
int fill();
extern int z[3][2];

int main()
{
    if (at(0, 0) != 1) return 1;
    if (at(1, 2) != 6) return 2;
    if (at(1, 0) != 4) return 3;
    if (z[2][1] != 0) return 4;
    if (fill() != 21 + 10 + 0) return 5;
    if (z[2][1] != 21) return 6;
    return 0;
}
