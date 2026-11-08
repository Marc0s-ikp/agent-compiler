int next();
int next_other();
int starts_zero();
int recurse(int d);
int in_loop();

int main()
{
    if (next() != 1) return 1;
    if (next() != 2) return 2;
    if (next_other() != 101) return 3;
    if (next() != 3) return 4;
    if (next_other() != 102) return 5;
    if (starts_zero() != 12) return 6;
    if (starts_zero() != 24) return 7;
    if (recurse(3) != 4) return 8;
    if (recurse(0) != 5) return 9;
    if (in_loop() != 13) return 10;
    if (in_loop() != 16) return 11;
    return 0;
}
