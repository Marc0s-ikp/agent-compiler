int s_abc(void);
int s_empty(void);
int s_concat(void);
int s_escape(void);
int s_nul(void);
int s_ptr(void);
int s_array(void);
int s_len(void);

int main()
{
    if (s_abc() != 4) return 1;
    if (s_empty() != 1) return 2;
    if (s_concat() != 4) return 3;
    if (s_escape() != 3) return 4;
    if (s_nul() != 4) return 5;
    if (s_ptr() != 4) return 6;
    if (s_array() != 6) return 7;
    if (s_len() != 5) return 8;
    return 0;
}
