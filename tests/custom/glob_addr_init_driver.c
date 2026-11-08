int combine();
int via_p();
int via_ptrs();
int is_null();
int same();
extern int x;
extern int *p;

int main()
{
    if (combine() != 659) return 1;
    if (p != &x) return 2;
    if (via_p() != 20) return 3;
    if (x != 20) return 4;
    if (via_ptrs() != 26) return 5;
    if (!is_null()) return 6;
    if (!same()) return 7;
    return 0;
}
