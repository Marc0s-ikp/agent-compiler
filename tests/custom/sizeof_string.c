int s_abc(void)     { return sizeof("abc"); }
int s_empty(void)   { return sizeof(""); }
int s_concat(void)  { return sizeof("a" "bc"); }
int s_escape(void)  { return sizeof("a\n"); }
int s_nul(void)     { return sizeof("a\0b"); }
int s_ptr(void)     { char *p; p = "abcdef"; return sizeof(p); }
int s_array(void)   { char s[] = "hello"; return sizeof(s); }
int s_len(void)     { char s[] = "hello"; return sizeof(s) - 1; }
