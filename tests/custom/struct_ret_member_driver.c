int member_of_call(void);
int member_of_big_call(void);
int call_as_arg(void);
int big_call_as_arg(void);
int return_param(void);
int return_big_param(void);
int param_returned_unchanged(void);

int main()
{
    if (member_of_call() != 4) return 1;
    if (member_of_big_call() != 7) return 2;
    if (call_as_arg() != 67) return 3;
    if (big_call_as_arg() != 123) return 4;
    if (return_param() != 8) return 5;
    if (return_big_param() != 234) return 6;
    if (param_returned_unchanged() != 152) return 7;
    return 0;
}
