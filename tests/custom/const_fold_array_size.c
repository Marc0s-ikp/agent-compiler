/* Wrapped/unsigned constants used as array sizes (checked with sizeof). */
char g_arr[(~0u >> 28) + 1];
int size_g() { return sizeof(g_arr); }
int size_shr() { char a[(~0u >> 28) + 1]; return sizeof(a); }
int size_wrap() { char a[65536 * 65536 + 3]; return sizeof(a); }
int size_mod() { char a[(0u - 1) % 7 + 1]; return sizeof(a); }
int size_sar() { char a[(-16 >> 2) + 10]; return sizeof(a); }
int size_div() { char a[(1 << 31) / (-2147483647 - 1) + 1]; return sizeof(a); }
