int no_default(int x);
int skipped_prelude(int x);
int empty_body(int x);
int negative_cases(int x);

int main()
{
    if (no_default(1) != 1) return 1;
    if (no_default(2) != 2) return 2;
    if (no_default(3) != 7) return 3;
    if (no_default(0) != 7) return 4;
    if (skipped_prelude(1) != 1) return 5;
    if (skipped_prelude(2) != 0) return 6;
    if (empty_body(9) != 9) return 7;
    if (negative_cases(-1) != 1) return 8;
    if (negative_cases(-2) != 2) return 9;
    if (negative_cases(0) != 3) return 10;
    if (negative_cases(5) != 4) return 11;
    return 0;
}
