int sparse(int x);
int once(int x);
int expr_switch(int x);

int main()
{
    if (sparse(1000) != 1) return 1;
    if (sparse(-1000) != 2) return 2;
    if (sparse(100000) != 3) return 3;
    if (sparse(2047) != 4) return 4;
    if (sparse(2048) != 5) return 5;
    if (sparse(-2049) != 6) return 6;
    if (sparse(0x7fffffff) != 7) return 7;
    if (sparse(999) != 0) return 8;
    if (once(0) != 101) return 9;
    if (once(1) != 201) return 10;
    if (once(5) != 301) return 11;
    if (expr_switch(3) != 1) return 12;
    if (expr_switch(4) != 2) return 13;
    if (expr_switch(0) != 0) return 14;
    return 0;
}
