/* An enum value that only fits in an int after 32-bit wraparound. */
enum { E_WRAP = 65536 * 65536 + 5, E_NEXT };
int e_wrap() { return E_WRAP; }
int e_next() { return E_NEXT; }
