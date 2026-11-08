char *joined(void);
char *joined_lines(void);
int joined_len(void);
int joined_escape(void);
int strcmp(const char *a, const char *b);

int main()
{
    if (strcmp(joined(), "abcde") != 0) return 1;
    if (strcmp(joined_lines(), "one two three") != 0) return 2;
    if (joined_len() != 4) return 3;
    if (joined_escape() != 'b') return 4;
    return 0;
}
