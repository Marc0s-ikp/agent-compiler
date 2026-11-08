int local_hides();
int param_hides(int g);
int global_after();
int set_global(int v);

int main()
{
    if (local_hides() != 1) return 1;
    if (param_hides(7) != 14) return 2;
    if (global_after() != 5) return 3;
    if (set_global(9) != 9) return 4;
    if (global_after() != 9) return 5;
    return 0;
}
