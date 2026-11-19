/* A4: global pointers initialised to the address of a global, an element, or a string. */
int a[5] = {10, 20, 30, 40, 50};
int *p_a = a;
int *p_a3 = &a[3];
int *p_a_plus = a + 2;
int *p_end = &a[5];
char text[8] = "abcdefg";
char *p_text = text;
char *p_text5 = &text[5];
char *p_text_plus = text + 2;
char *p_str = "hello";
int *null_ptr = 0;
int *p_self = &a[0];
int *table[3] = {&a[0], &a[2], &a[4]};
static int sa[3] = {1, 2, 3};
int *p_sa1 = &sa[1];

int get_a3(void) { return *p_a3; }
int get_plus(void) { return *p_a_plus; }
int get_text5(void) { return *p_text5; }
int get_text_plus(void) { return *p_text_plus; }
int get_str1(void) { return p_str[1]; }
int get_table(int i) { return *table[i]; }
int get_sa1(void) { return *p_sa1; }
int is_null(void) { return null_ptr == 0; }

/* the addresses themselves are compared with the driver's own */
int *addr_p_a(void) { return p_a; }
int *addr_p_a3(void) { return p_a3; }
int *addr_p_a_plus(void) { return p_a_plus; }
int *addr_p_end(void) { return p_end; }
char *addr_p_text5(void) { return p_text5; }
char *addr_p_text_plus(void) { return p_text_plus; }
int *addr_table(int i) { return table[i]; }
int *addr_a(void) { return a; }
char *addr_text(void) { return text; }
