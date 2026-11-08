int oct1(void) { return '\1'; }
int oct3(void) { return '\101'; }   /* 65 */
int oct_hi(void) { return '\377'; } /* 255, plain char is unsigned */
int hex1(void) { return '\x41'; }   /* 65 */
int hex_hi(void) { return '\xff'; } /* 255 */
int hex_lo(void) { return '\x0a'; } /* 10 */
int type_is_int(void) { return sizeof('a'); } /* 4 in C */
