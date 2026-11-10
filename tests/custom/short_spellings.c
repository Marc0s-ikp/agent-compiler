/* Every spelling of short, as parameter, return type and local. */
short r1(short a) { return a; }
short int r2(short int a) { return a; }
signed short r3(signed short a) { return a; }
signed short int r4(signed short int a) { return a; }
unsigned short r5(unsigned short a) { return a; }
unsigned short int r6(unsigned short int a) { return a; }

int locals(int v)
{
    short a = v;
    short int b = v;
    signed short c = v;
    signed short int d = v;
    unsigned short e = v;
    unsigned short int f = v;
    /* signed ones agree with each other, unsigned ones agree with each other */
    if (a != b || b != c || c != d) return -1;
    if (e != f) return -2;
    return a * 2 + (e != a);
}
int sizes(void)
{
    return sizeof(short) + sizeof(short int) * 10 + sizeof(signed short) * 100
         + sizeof(signed short int) * 1000 + sizeof(unsigned short) * 10000
         + sizeof(unsigned short int) * 100000;
}
