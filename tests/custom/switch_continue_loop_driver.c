int run(int n);
int while_continue(int n);
int break_in_inner_loop(int x);
int do_while_continue(int n);

int main()
{
    if (run(6) != 422) return 1;
    if (run(1) != 0) return 2;
    if (run(2) != 101) return 3;
    if (while_continue(5) != 3) return 4;
    if (break_in_inner_loop(1) != 103) return 5;
    if (break_in_inner_loop(2) != 5) return 6;
    if (do_while_continue(6) != 12) return 7;
    return 0;
}
