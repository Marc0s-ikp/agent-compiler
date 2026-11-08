int plain(void);
int newline(void);
int tab(void);
int nul(void);
int bslash(void);
int squote(void);
int dquote(void);
int dquote2(void);
int digit(void);
int arith(void);
int cmp(char c);
int space(void);

int main()
{
    if (plain() != 97) return 1;
    if (newline() != 10) return 2;
    if (tab() != 9) return 3;
    if (nul() != 0) return 4;
    if (bslash() != 92) return 5;
    if (squote() != 39) return 6;
    if (dquote() != 34) return 7;
    if (dquote2() != 34) return 8;
    if (digit() != 7) return 9;
    if (arith() != 25) return 10;
    if (cmp('m') != 1) return 11;
    if (cmp('A') != 0) return 12;
    if (space() != 32) return 13;
    return 0;
}
