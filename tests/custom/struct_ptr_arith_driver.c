int arrow(void); int deref_dot(void); int stride(void); int walk(void); int diff(void);
int increment_deref(void); int member_address(void); int via_function(void);
int pointer_to_pointer(void);

int main()
{
    if (arrow() != 456) return 1;
    if (deref_dot() != 14) return 2;
    if (stride() != 48) return 3;
    if (walk() != 1234) return 4;
    if (diff() != 5) return 5;
    if (increment_deref() != 131) return 6;
    if (member_address() != 123) return 7;
    if (via_function() != 33) return 8;
    if (pointer_to_pointer() != 98) return 9;
    return 0;
}
