int f(int x);
int later(int x);

/* defined here: if the compiler emitted a label or storage for the unused
   prototype in prototype_only.c, linking would fail with a duplicate symbol */
int unused(int x)
{
    return x;
}

int main()
{
    if (f(5) != 11) return 1;
    if (unused(3) != 3) return 2;
    return 0;
}
