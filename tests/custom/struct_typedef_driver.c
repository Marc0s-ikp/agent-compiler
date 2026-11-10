int point(void); int segment(void); int with_tag(void); int node(void);
int ptr_typedef(void); int array_typedef(void); int by_value(void); int sizes(void);

int main()
{
    if (point() != 23) return 1;
    if (segment() != 4321) return 2;
    if (with_tag() != 6) return 3;
    if (node() != 2) return 4;
    if (ptr_typedef() != 89) return 5;
    if (array_typedef() != 24321) return 6;
    if (by_value() != 67) return 7;
    if (sizes() != 964) return 8;
    return 0;
}
