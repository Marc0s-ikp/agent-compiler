int run(void);
int head(void);
int size(void);

int main()
{
    if (run() != 0) return 1;
    if (head() != 'a' + 'b' + 'c') return 2;
    if (size() != 10) return 3;
    return 0;
}
