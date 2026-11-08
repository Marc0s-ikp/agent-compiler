int val(int which);
int pick(int e);
int variable_use();

int main()
{
    if (val(0) != 0) return 1;
    if (val(1) != 1) return 2;
    if (val(2) != 10) return 3;
    if (val(3) != 11) return 4;
    if (val(4) != -1) return 5;
    if (val(5) != 0) return 6;
    if (val(6) != 6) return 7;
    if (val(7) != 7) return 8;
    if (pick(0) != 10) return 9;
    if (pick(5) != 5) return 10;
    if (variable_use() != 12) return 11;
    return 0;
}
