int my_strlen(char *s);
void my_strcpy(char *d, char *s);
int my_strcmp(char *a, char *b);
char *my_strchr(char *s, int c);
int copy_and_measure(void);
int literal_len(void);

int main()
{
    char b[8];
    if (my_strlen("hello") != 5) return 1;
    my_strcpy(b, "abc");
    if (b[0] != 'a' || b[2] != 'c' || b[3] != 0) return 2;
    if (my_strcmp("abc", "abc") != 0) return 3;
    if (my_strcmp("abc", "abd") >= 0) return 4;
    if (my_strcmp("b", "a") <= 0) return 5;
    if (my_strchr("hello", 'l') == 0 || *my_strchr("hello", 'l') != 'l') return 6;
    if (my_strchr("hello", 'z') != 0) return 7;
    if (copy_and_measure() != 700 + '!') return 8;
    if (literal_len() != 4) return 9;
    return 0;
}
