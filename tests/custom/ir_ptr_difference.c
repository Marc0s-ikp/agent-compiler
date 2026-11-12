/* Pointer difference: element sizes 1, 2, 4 and 12 (not a power of two), in
   both directions, and a difference used inside a larger expression. */
int idiff(int *a, int *b) { return a - b; }
int cdiff(char *a, char *b) { return a - b; }
int sdiff(short *a, short *b) { return a - b; }
int rows(int (*p)[3], int (*q)[3]) { return q - p; }
int mixed(int *a) { return (a + 7) - (a + 2) + ((a + 2) - (a + 7)) * 10; }
