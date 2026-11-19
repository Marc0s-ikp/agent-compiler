#include <stddef.h>
extern int a[5];
extern char text[8];
int get_a3(void), get_plus(void), get_text5(void), get_text_plus(void), get_str1(void);
int get_table(int i), get_sa1(void), is_null(void);
int *addr_p_a(void), *addr_p_a3(void), *addr_p_a_plus(void), *addr_p_end(void);
char *addr_p_text5(void), *addr_p_text_plus(void);
int *addr_table(int i);
int *addr_a(void);
char *addr_text(void);

int main(void)
{
    if (addr_a() != a || addr_text() != text) return 1;
    if (addr_p_a() != &a[0]) return 2;
    if (addr_p_a3() != &a[3]) return 3;
    if (addr_p_a_plus() != a + 2) return 4;
    if (addr_p_end() != &a[5]) return 5;
    if (addr_p_text5() != &text[5]) return 6;
    if (addr_p_text_plus() != text + 2) return 7;
    if (addr_table(0) != &a[0] || addr_table(1) != &a[2] || addr_table(2) != &a[4]) return 8;
    if (get_a3() != a[3] || get_plus() != a[2]) return 9;
    if (get_text5() != text[5] || get_text_plus() != text[2]) return 10;
    if (get_str1() != "hello"[1]) return 11;
    if (get_table(0) != a[0] || get_table(1) != a[2] || get_table(2) != a[4]) return 12;
    if (get_sa1() != 2) return 13;
    if (!is_null()) return 14;
    return 0;
}
