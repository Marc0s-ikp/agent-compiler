# ABI check results

Final `abi_check` run of the release (2026-11-19), on `release/two-backends` after the probe stub fix. Seeds 606, 707, 808, 909 and
1010, 500 random signatures each, on both targets. Each seed is built seven ways (ours to ours; ours to GCC, GCC to ours and GCC to GCC
at `-O0` and at `-O2`) and every build ran with exit code 0 and 0 failures: 35 of 35 runs per target. Each cell is
*signatures / occurrences*: how many of the seed's 500 signatures hit the case, and how many arguments or results did. A case
with no hits would be untested; every case was hit on every seed. Case descriptions are those `gen_abi.py` prints.

The model behind the aarch64 cases was itself checked against GCC's `-O2` caller with `probe_aapcs64.py` on the same seeds
(606 to 1010 x 500): 0 of 2500 signatures disagree, and every case was hit.

Reproduce with the commands in `README.md` ("Build and run").

## riscv32 (17 cases)

| Case | 606 | 707 | 808 | 909 | 1010 | Total | What it is |
|---|---:|---:|---:|---:|---:|---:|---|
| `a` | 114 / 152 | 101 / 125 | 110 / 146 | 113 / 159 | 103 / 136 | 541 / 718 | int-convention struct <= 4 bytes in one a register |
| `b` | 179 / 271 | 173 / 283 | 175 / 283 | 181 / 283 | 175 / 280 | 883 / 1400 | 5-8 byte struct in two a registers |
| `c` | 114 / 114 | 123 / 123 | 113 / 113 | 107 / 107 | 122 / 122 | 579 / 579 | small struct split across a7 and the stack |
| `d` | 153 / 269 | 153 / 269 | 144 / 264 | 150 / 240 | 153 / 246 | 753 / 1288 | small struct entirely on the stack |
| `e` | 146 / 196 | 128 / 182 | 133 / 186 | 134 / 183 | 145 / 202 | 686 / 949 | float+float struct in two fa registers |
| `f` | 131 / 190 | 113 / 155 | 124 / 187 | 110 / 158 | 111 / 182 | 589 / 872 | float+int / int+float struct in fa + a |
| `g` | 58 / 67 | 70 / 83 | 63 / 77 | 64 / 74 | 56 / 62 | 311 / 363 | single float/double field struct in one fa |
| `h` | 132 / 187 | 151 / 221 | 140 / 217 | 135 / 203 | 156 / 225 | 714 / 1053 | FP-eligible struct, fa or a ran out: integer convention |
| `i_reg` | 92 / 117 | 86 / 118 | 81 / 106 | 93 / 111 | 97 / 122 | 449 / 574 | >8-byte struct by reference, pointer in a register |
| `i_stack` | 69 / 86 | 74 / 89 | 72 / 85 | 81 / 95 | 78 / 91 | 374 / 446 | >8-byte struct by reference, pointer on the stack |
| `j` | 38 / 38 | 28 / 28 | 32 / 32 | 37 / 37 | 41 / 41 | 176 / 176 | struct return > 8 bytes via hidden pointer |
| `k` | 157 / 157 | 176 / 176 | 175 / 175 | 150 / 150 | 172 / 172 | 830 / 830 | small struct return in a0/a1 |
| `l` | 166 / 166 | 165 / 165 | 154 / 154 | 166 / 166 | 169 / 169 | 820 / 820 | FP struct return in fa0/fa1 |
| `m` | 150 / 180 | 141 / 171 | 139 / 178 | 146 / 192 | 155 / 186 | 731 / 907 | union containing floats (integer convention) |
| `n` | 87 / 241 | 107 / 309 | 103 / 285 | 93 / 265 | 122 / 314 | 512 / 1414 | struct argument after fa0-fa7 are used up |
| `x_dsplit` | 38 / 38 | 56 / 56 | 53 / 53 | 55 / 55 | 43 / 43 | 245 / 245 | (scalar) double split across a7 and the stack |
| `x_pad8` | 23 / 23 | 21 / 21 | 20 / 20 | 19 / 20 | 16 / 16 | 99 / 100 | 8-aligned value on the stack after a padding word |

## aarch64 (58 cases)

| Case | 606 | 707 | 808 | 909 | 1010 | Total | What it is |
|---|---:|---:|---:|---:|---:|---:|---|
| `x_int` | 322 / 590 | 330 / 618 | 326 / 627 | 317 / 605 | 311 / 611 | 1606 / 3051 | 4-byte integer in an x register (C.9) |
| `x_ptr` | 310 / 585 | 300 / 573 | 299 / 561 | 309 / 580 | 292 / 515 | 1510 / 2814 | pointer in an x register (C.9) |
| `x_narrow` | 409 / 1509 | 420 / 1650 | 397 / 1476 | 408 / 1532 | 404 / 1530 | 2038 / 7697 | char/short in an x register (C.9; upper bits unspecified) |
| `x_last` | 246 / 246 | 265 / 265 | 246 / 246 | 257 / 257 | 234 / 234 | 1248 / 1248 | the eighth integer or pointer argument, in x7 |
| `x_after_v` | 56 / 110 | 54 / 106 | 48 / 82 | 62 / 112 | 61 / 135 | 281 / 545 | integer or pointer in an x register once v0-v7 are used up (NSRN 8, NGRN < 8) |
| `v_float` | 260 / 603 | 242 / 579 | 232 / 582 | 273 / 704 | 262 / 683 | 1269 / 3151 | float in an s register (C.1) |
| `v_double` | 277 / 690 | 245 / 585 | 255 / 631 | 258 / 675 | 263 / 666 | 1298 / 3247 | double in a d register (C.1) |
| `v_last` | 87 / 87 | 76 / 76 | 84 / 84 | 96 / 96 | 104 / 104 | 447 / 447 | the eighth float or double argument, in v7 (C.1) |
| `v_after_x` | 132 / 201 | 139 / 206 | 117 / 176 | 120 / 175 | 125 / 184 | 633 / 942 | float or double in a v register once x0-x7 are used up (NGRN 8, NSRN < 8) |
| `s_first` | 357 / 357 | 366 / 366 | 366 / 366 | 389 / 389 | 367 / 367 | 1845 / 1845 | the first stack argument (NSAA 0) (C.6 or C.14, C.17) |
| `s_int` | 63 / 84 | 65 / 93 | 65 / 88 | 80 / 105 | 61 / 86 | 334 / 456 | 4-byte integer on the stack (C.14, C.17) |
| `s_ptr` | 96 / 154 | 109 / 191 | 89 / 147 | 108 / 177 | 105 / 170 | 507 / 839 | pointer on the stack (C.14, C.17) |
| `s_narrow` | 131 / 283 | 161 / 354 | 146 / 366 | 168 / 409 | 157 / 406 | 763 / 1818 | char/short on the stack, 8-byte slot (C.16) |
| `s_float` | 66 / 115 | 56 / 86 | 71 / 114 | 76 / 131 | 74 / 113 | 343 / 559 | float on the stack, 8-byte slot (C.5, C.6) |
| `s_double` | 71 / 104 | 59 / 96 | 72 / 102 | 80 / 131 | 76 / 115 | 358 / 548 | double on the stack (C.6) |
| `s_int_vfree` | 184 / 502 | 189 / 568 | 177 / 543 | 201 / 616 | 184 / 610 | 935 / 2839 | integer or pointer on the stack while v registers are free (NGRN 8, NSRN < 8) |
| `s_fp_xfree` | 89 / 207 | 68 / 164 | 90 / 198 | 95 / 249 | 98 / 214 | 440 / 1032 | float or double on the stack while x registers are free (NSRN 8, NGRN < 8) |
| `s_mixed` | 110 / 161 | 117 / 171 | 124 / 164 | 113 / 160 | 121 / 196 | 585 / 852 | a stack argument of the other class than the one before it (one NSAA for both) |
| `s_next` | 275 / 682 | 281 / 755 | 276 / 751 | 307 / 853 | 288 / 818 | 1427 / 3859 | a stack argument at NSAA > 0 (slots advance by 8) |
| `s_odd` | 196 / 196 | 178 / 178 | 208 / 208 | 204 / 204 | 200 / 200 | 986 / 986 | stack area not a multiple of 16 before padding (SP alignment) |
| `r_int` | 35 / 35 | 41 / 41 | 32 / 32 | 48 / 48 | 48 / 48 | 204 / 204 | 4-byte integer result in w0 |
| `r_ptr` | 32 / 32 | 56 / 56 | 40 / 40 | 39 / 39 | 28 / 28 | 195 / 195 | pointer result in x0 |
| `r_narrow` | 92 / 92 | 98 / 98 | 105 / 105 | 95 / 95 | 93 / 93 | 483 / 483 | char/short result in w0 (upper bits unspecified) |
| `r_float` | 24 / 24 | 15 / 15 | 21 / 21 | 21 / 21 | 18 / 18 | 99 / 99 | float result in s0 (6.9: where void f(T) gets its argument) |
| `r_double` | 20 / 20 | 20 / 20 | 19 / 19 | 17 / 17 | 27 / 27 | 103 / 103 | double result in d0 (6.9) |
| `h_v1` | 28 / 29 | 24 / 24 | 20 / 25 | 19 / 19 | 34 / 34 | 125 / 131 | HFA of 1 member in v registers (C.2) |
| `h_v2` | 52 / 63 | 81 / 93 | 65 / 79 | 51 / 63 | 69 / 76 | 318 / 374 | HFA of 2 members in v registers (C.2) |
| `h_v3` | 29 / 30 | 36 / 36 | 32 / 37 | 23 / 24 | 29 / 29 | 149 / 156 | HFA of 3 members in v registers (C.2) |
| `h_v4` | 42 / 44 | 38 / 41 | 35 / 35 | 25 / 26 | 51 / 51 | 191 / 197 | HFA of 4 members in v registers (C.2) |
| `h_float` | 79 / 96 | 98 / 114 | 84 / 107 | 67 / 87 | 93 / 118 | 421 / 522 | HFA of floats in v registers |
| `h_double` | 61 / 70 | 65 / 80 | 60 / 69 | 40 / 45 | 59 / 72 | 285 / 336 | HFA of doubles in v registers |
| `h_nested` | 49 / 52 | 54 / 55 | 39 / 40 | 31 / 34 | 36 / 39 | 209 / 220 | HFA built with a nested record, in v registers |
| `h_array` | 57 / 60 | 57 / 63 | 56 / 67 | 35 / 40 | 73 / 82 | 278 / 312 | HFA built with an array, in v registers |
| `h_union` | 41 / 45 | 41 / 44 | 40 / 46 | 25 / 26 | 43 / 47 | 190 / 208 | HFA that is or holds a union, in v registers |
| `h_big_v` | 39 / 41 | 40 / 41 | 31 / 32 | 21 / 23 | 33 / 34 | 164 / 171 | HFA over 16 bytes by value in v registers (B.3 before B.4) |
| `h_last` | 11 / 11 | 11 / 11 | 10 / 10 | 9 / 9 | 9 / 9 | 50 / 50 | HFA ending in v7 (NSRN + members = 8) |
| `h_stack` | 37 / 51 | 26 / 39 | 41 / 55 | 36 / 51 | 41 / 58 | 181 / 254 | HFA on the stack, v registers used up before it (NSRN 8) |
| `h_stack_part` | 34 / 34 | 32 / 32 | 41 / 41 | 46 / 46 | 37 / 37 | 190 / 190 | HFA on the stack while NSRN < 8 but too few v registers left (C.3 sets NSRN 8) |
| `h_stack_fp` | 30 / 54 | 31 / 62 | 39 / 76 | 38 / 71 | 33 / 64 | 171 / 327 | float or double on the stack after C.3 set NSRN 8, though v[NSRN] was free before |
| `h_big_stack` | 23 / 26 | 18 / 21 | 24 / 28 | 24 / 28 | 23 / 25 | 112 / 128 | HFA over 16 bytes on the stack by value |
| `c_x1` | 68 / 98 | 76 / 105 | 70 / 103 | 62 / 96 | 69 / 108 | 345 / 510 | non-HFA record of up to 8 bytes in one x register (C.12) |
| `c_x2` | 57 / 68 | 56 / 74 | 62 / 85 | 53 / 65 | 58 / 69 | 286 / 361 | non-HFA record of 9 to 16 bytes in two x registers (C.12) |
| `c_x_odd` | 37 / 44 | 34 / 42 | 43 / 47 | 33 / 39 | 44 / 51 | 191 / 223 | record of a size that is not 1, 2, 4, 8 or 16, in x registers |
| `c_x_float` | 50 / 64 | 62 / 80 | 58 / 84 | 57 / 73 | 56 / 70 | 283 / 371 | non-HFA record holding a float or double, in x registers |
| `c_x_union` | 18 / 20 | 23 / 26 | 14 / 14 | 18 / 18 | 14 / 15 | 87 / 93 | union in x registers |
| `c_x_last` | 6 / 6 | 7 / 7 | 12 / 12 | 7 / 7 | 7 / 7 | 39 / 39 | record ending in x7 |
| `c_stack_one` | 51 / 51 | 55 / 55 | 52 / 52 | 48 / 48 | 54 / 54 | 260 / 260 | 9-16 byte record on the stack with one x register left (NGRN 7: C.13, no split) |
| `c_stack_int` | 48 / 84 | 52 / 109 | 51 / 92 | 46 / 100 | 53 / 114 | 250 / 499 | integer or pointer on the stack after C.13 left x7 unused |
| `c_stack_full` | 92 / 137 | 103 / 154 | 95 / 137 | 80 / 125 | 90 / 130 | 460 / 683 | record on the stack once x0-x7 are used up (NGRN 8) |
| `c_stack_odd` | 31 / 33 | 40 / 45 | 35 / 39 | 33 / 39 | 31 / 32 | 170 / 188 | odd-sized record on the stack followed by another stack argument (B.5 rounding) |
| `ref_x` | 18 / 20 | 32 / 37 | 22 / 26 | 19 / 21 | 20 / 22 | 111 / 126 | record over 16 bytes (not an HFA) by reference, pointer in an x register |
| `ref_stack` | 23 / 26 | 21 / 21 | 15 / 15 | 19 / 19 | 16 / 16 | 94 / 97 | record over 16 bytes (not an HFA) by reference, pointer on the stack |
| `r_rec_x1` | 94 / 94 | 81 / 81 | 83 / 83 | 85 / 85 | 89 / 89 | 432 / 432 | record result of up to 8 bytes in x0 |
| `r_rec_x2` | 49 / 49 | 63 / 63 | 52 / 52 | 53 / 53 | 53 / 53 | 270 / 270 | record result of 9 to 16 bytes in x0/x1 |
| `r_hfa` | 99 / 99 | 73 / 73 | 79 / 79 | 74 / 74 | 89 / 89 | 414 / 414 | HFA result in v registers |
| `r_hfa4` | 30 / 30 | 16 / 16 | 34 / 34 | 17 / 17 | 27 / 27 | 124 / 124 | 4-member HFA result in v0-v3 |
| `r_mem` | 39 / 39 | 34 / 34 | 53 / 53 | 50 / 50 | 39 / 39 | 215 / 215 | record result in memory, address in x8 |
| `r_mem_xfull` | 32 / 32 | 27 / 27 | 43 / 43 | 39 / 39 | 32 / 32 | 173 / 173 | x8 result while x0-x7 are all used by arguments |
