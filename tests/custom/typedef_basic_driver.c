int twice(int x);
int via_pointer(int *p);
int array_type();
int pointer_array();
int ret_ptr_use();
extern int gl[3];

int main()
{
    int v = 9;
    if (twice(21) != 42) return 1;
    if (via_pointer(&v) != 10) return 2;
    if (v != 10) return 3;
    if (array_type() != 6) return 4;
    if (gl[2] != 6) return 5;
    if (pointer_array() != 45) return 6;
    if (ret_ptr_use() != 8) return 7;
    return 0;
}
