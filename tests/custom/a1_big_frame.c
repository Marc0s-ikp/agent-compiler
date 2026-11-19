/* A frame bigger than a 12-bit immediate and than the scaled 12-bit load/store
   offsets of AArch64: the locals and temps sit far from the frame pointer, so
   every access goes through the large-offset fallback (D27, 27.3). */
int f()
{
    char pad[70000];
    int x = 5;
    int y = x * 3;
    char c = 9;
    short s = -2;
    return (x + y - 20) + (c - 9) + (s + 2) + 7;
}
