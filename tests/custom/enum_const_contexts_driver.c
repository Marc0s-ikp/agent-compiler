int fill();
int get(int i);
int tern(int x);

int main()
{
    if (get(0) != 1) return 1;
    if (get(1) != 4) return 2;
    if (get(2) != 8) return 3;
    if (get(3) != -4) return 4;
    if (fill() != 414) return 5;
    if (tern(1) != 4) return 6;
    if (tern(0) != 8) return 7;
    return 0;
}
