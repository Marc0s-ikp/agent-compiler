int size_s(void);
int size_t_(void);
int t_tail(void);
int modify(void);
int sum_u(void);
int v_tail(void);
int w_len(void);
extern char s[3];
extern char t[6];

int main()
{
    if (size_s() != 3) return 1;
    if (size_t_() != 6) return 2;
    if (s[0] != 'a' || s[1] != 'b' || s[2] != 0) return 3;
    if (t[0] != 'h' || t[1] != 'i') return 4;
    if (t_tail() != 0) return 5;
    if (sum_u() != 6) return 6;
    if (v_tail() != 97 * 10000 + 98 * 100) return 7;
    if (w_len() != 7) return 8;
    if (modify() != 'z' * 256 + 'b') return 9;
    if (s[0] != 'z') return 10;
    return 0;
}
