int array_sum(void); int through_pointer(void); int ptr_arith(void);
int stride(void); int via_function(void);

int main()
{
    if (array_sum() != 60) return 1;
    if (through_pointer() != 263) return 2;
    if (ptr_arith() != 232) return 3;
    if (stride() != 8) return 4;
    if (via_function() != 84) return 5;
    return 0;
}
