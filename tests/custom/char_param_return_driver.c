char ret_char(int v);
int take_char(char c);
int pass_through(int v);
char many(char a, char b, char c, char d, char e, char f, char g, char h, char i, char j);

int main()
{
    if (ret_char(65) != 65) return 1;
    if (ret_char(321) != 65) return 2;
    if (take_char(200) != 200) return 3;
    if (take_char(456) != 200) return 4;
    if (pass_through(513) != 1) return 5;
    if (many(1, 2, 3, 4, 5, 6, 7, 8, 9, 10) != 55) return 6;
    if (many(100, 100, 100, 0, 0, 0, 0, 0, 0, 0) != 44) return 7;
    return 0;
}
