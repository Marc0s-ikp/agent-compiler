extern int t[5];
int sum_t();
int tail_u();
int last_v();
int get_w(int i);

int main()
{
    if (t[4] != 5) return 1;
    if (sum_t() != 15) return 2;
    if (tail_u() != 78000) return 3;
    if (last_v() != 30) return 4;
    if (get_w(0) != -1) return 5;
    if (get_w(1) != 16) return 6;
    if (get_w(2) != 2) return 7;
    t[0] = 100;
    if (sum_t() != 114) return 8;
    return 0;
}
