char *esc(void);
char *with_nul(void);
int nul_then_more(void);
char *quote_chars(void);
char *squote(void);

int main()
{
    char *e;
    e = esc();
    if (e[0] != 'a' || e[1] != 9 || e[2] != 'b' || e[3] != 10) return 1;
    if (e[4] != '\\' || e[5] != '"' || e[6] != '\'') return 2;
    if (e[7] != 'A' || e[8] != 'B' || e[9] != 0) return 3;
    if (with_nul()[2] != 0 || with_nul()[3] != 'c' || with_nul()[5] != 0) return 4;
    if (nul_then_more() != 'c') return 5;
    if (quote_chars()[0] != '"' || quote_chars()[1] != 'q' || quote_chars()[2] != '"' || quote_chars()[3] != 0) return 6;
    if (squote()[0] != '\'' || squote()[1] != 0) return 7;
    return 0;
}
