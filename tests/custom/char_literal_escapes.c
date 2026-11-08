int plain(void)   { return 'a'; }
int newline(void) { return '\n'; }
int tab(void)     { return '\t'; }
int nul(void)     { return '\0'; }
int bslash(void)  { return '\\'; }
int squote(void)  { return '\''; }
int dquote(void)  { return '"'; }
int dquote2(void) { return '\"'; }
int digit(void)   { return '7' - '0'; }
int arith(void)   { return 'z' - 'a'; }
int cmp(char c)   { return c >= 'a' && c <= 'z'; }
int space(void)   { return ' '; }
