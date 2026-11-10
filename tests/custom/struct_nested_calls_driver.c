int two_temps(void);
int two_big_temps(void);
int nested(void);
int across_call(void);
int recurse(void);
int recurse_big(void);

int main()
{
    if (two_temps() != 4221) return 1;
    if (two_big_temps() != 1206) return 2;
    if (nested() != 2121) return 3;
    if (across_call() != 60) return 4;
    if (recurse() != 532) return 5;
    if (recurse_big() != 100000 + 2000 + 30) return 6;
    return 0;
}
