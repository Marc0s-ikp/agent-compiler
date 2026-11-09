double call_split(void);
int call_split_next(void);

int main(void)
{
    if (call_split() != 25.0 + 100 + 1 + 7) return 1;
    if (call_split_next() != 97) return 2;
    return 0;
}
