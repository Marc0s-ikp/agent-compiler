int mutual(void); int use_prototype(void); int define_after_pointer(void);
int declare_and_define_variable(void);

int main()
{
    if (mutual() != 43) return 1;
    if (use_prototype() != 30) return 2;
    if (define_after_pointer() != 77) return 3;
    if (declare_and_define_variable() != 8) return 4;
    return 0;
}
