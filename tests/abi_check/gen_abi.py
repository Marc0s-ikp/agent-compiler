#!/usr/bin/env python3
"""Random ABI cross-check generator (integer, pointer, float, double, struct and union signatures).

Prints, per run, how many signatures hit each ABI case: the ilp32d cases (classify) for riscv32, the
AAPCS64 cases (classify_aapcs64) for aarch64.

Writes three C files into OUT_DIR:
  callee.c  N functions f<i>(params) -> ret. Each checks every parameter
            against its expected value and calls abi_fail(i, k) on a mismatch,
            then returns a value derived from the parameters.
  caller.c  N functions caller<i>(void) that call f<i> with the expected values
            (as literals, as same-type locals, or as int locals converted at the
            call) and check the return value.
  driver.c  main, abi_fail, abi_ptr_c, abi_ptr_i, abi_int (always built by GCC).
Usage: gen_abi.py SEED COUNT OUT_DIR [riscv32|aarch64]   (default riscv32)
"""
import collections
import random
import struct
import sys

INT_TYPES = {
    # name: (size, signed)
    "int": (4, True), "unsigned": (4, False), "char": (1, False),
    "signed char": (1, True), "unsigned char": (1, False),
    "short": (2, True), "unsigned short": (2, False),
}
PTR_TYPES = ["int *", "char *"]
FLOAT_TYPES = ["float", "double"]
# Structs and unions (step 6). Each is (kind, fields); a field is (name, type)
# or (name, type, count) for an array; a field type is a scalar or an earlier
# record. Declarations, the checked leaves, the size, the alignment and the
# ilp32d class all come from this one table (see layout, flatten, classify).
COMMON_RECORDS = {
    "struct Sc1": ("struct", [("a", "char")]),
    "struct Sc3": ("struct", [("a", "char"), ("b", "char"), ("c", "char")]),
    "struct Sc5": ("struct", [("c", "char", 5)]),
    "struct Ss3": ("struct", [("a", "short"), ("b", "short"), ("c", "short")]),
    "struct Si1": ("struct", [("a", "int")]),
    "struct Sp8": ("struct", [("a", "int"), ("b", "int")]),
    "struct Sm8": ("struct", [("a", "char"), ("b", "int")]),
    "struct Sa2": ("struct", [("a", "int", 2)]),
    "struct Sbig": ("struct", [("a", "int"), ("b", "int"), ("c", "int")]),
    "struct Swide": ("struct", [("a", "int"), ("b", "int"), ("c", "int"), ("d", "int")]),
    "struct Sf1": ("struct", [("x", "float")]),
    "struct Sd1": ("struct", [("x", "double")]),
    "struct Sff": ("struct", [("a", "float"), ("b", "float")]),
    "struct Sfa2": ("struct", [("a", "float", 2)]),
    "struct Snest": ("struct", [("in", "struct Sf1"), ("b", "float")]),
    "struct Sfi": ("struct", [("f", "float"), ("i", "int")]),
    "struct Sif": ("struct", [("i", "int"), ("f", "float")]),
    "struct Sfs": ("struct", [("f", "float"), ("s", "short")]),
    "struct Sid": ("struct", [("i", "int"), ("d", "double")]),
    "struct Sdi": ("struct", [("d", "double"), ("i", "int")]),
    "struct Scd": ("struct", [("c", "signed char"), ("d", "double")]),
    "struct Sdd": ("struct", [("a", "double"), ("b", "double")]),
    "struct Sfd": ("struct", [("a", "float"), ("b", "double")]),
    "struct Scf": ("struct", [("c", "char"), ("f", "float")]),
    "struct Sfp": ("struct", [("f", "float"), ("p", "int *")]),    # a pointer: not floating convention
    "struct Sfff": ("struct", [("a", "float"), ("b", "float"), ("c", "float")]),  # three fields: by reference
    "union Uff": ("union", [("a", "float"), ("b", "float")]),
    "union Ufi": ("union", [("f", "float"), ("i", "int")]),
    "union Uud": ("union", [("d", "double"), ("i", "int")]),
    "struct Snu": ("struct", [("u", "union Uff"), ("g", "float")]),  # a union inside: integer convention
}
# Records only aarch64 draws (they are not in the riscv32 files): the shapes AAPCS64's record rules
# distinguish. HFAs of 1 to 4 members built as an array, a nested record, a union; HFAs over 16 bytes;
# things that are not HFAs (five floats, float with double, a union of float and double); odd sizes; records
# over 16 bytes (by reference); records holding pointers (8 bytes each on LP64).
AARCH64_RECORDS = {
    "struct Sf4": ("struct", [("a", "float", 4)]),                       # HFA of 4 floats, an array
    "struct Sd3": ("struct", [("a", "double"), ("b", "double"), ("c", "double")]),   # HFA of 3, 24 bytes
    "struct Sd4a": ("struct", [("a", "double", 4)]),                     # HFA of 4 doubles, 32 bytes
    "struct Sd4n": ("struct", [("x", "struct Sdd"), ("y", "struct Sdd")]),  # HFA of 4 doubles, nested
    "struct Sf4m": ("struct", [("a", "float", 2), ("n", "struct Sff")]),  # HFA of 4 floats, array and nested
    "union Udd": ("union", [("a", "double"), ("b", "double", 2)]),       # HFA of 2 doubles, a union
    "union Uf3": ("union", [("a", "float", 3), ("b", "float", 2)]),      # HFA of 3 floats, a union
    "struct Sf5": ("struct", [("a", "float", 5)]),                       # 5 floats: not an HFA, 20 bytes
    "union Ufd": ("union", [("f", "float"), ("d", "double")]),           # float and double: not homogeneous
    "struct Sc7": ("struct", [("c", "char", 7)]),
    "struct Sc11": ("struct", [("c", "char", 11)]),
    "struct Sc13": ("struct", [("c", "char", 13)]),
    "struct Si5": ("struct", [("a", "int", 5)]),                         # 20 bytes: by reference
    "struct Sddi": ("struct", [("a", "double"), ("b", "double"), ("c", "int")]),     # 24 bytes, not an HFA
    "union Ubig": ("union", [("a", "int", 5), ("d", "double")]),         # 24 bytes
    "struct Sp1": ("struct", [("p", "int *")]),                          # 8 bytes
    "struct Sp2": ("struct", [("p", "int *"), ("q", "char *")]),         # 16 bytes
    "struct Spi": ("struct", [("p", "char *"), ("i", "int")]),           # 16 bytes with 4 bytes of padding
}
RECORDS = dict(COMMON_RECORDS)  # the records of the current target (setup_target)
SCALAR_LAYOUT = {  # name: (size, align, class) with class i(nteger), p(ointer), f(loating)
    "int": (4, 4, "i"), "unsigned": (4, 4, "i"), "char": (1, 1, "i"), "signed char": (1, 1, "i"),
    "unsigned char": (1, 1, "i"), "short": (2, 2, "i"), "unsigned short": (2, 2, "i"),
    "int *": (4, 4, "p"), "char *": (4, 4, "p"), "float": (4, 4, "f"), "double": (8, 8, "f"),
}

# LP64 scalars of aarch64 (D27 27.1): pointers are 8 bytes. Plain char is unsigned on both targets.
# Only what the compiler supports so far (A3: integers and pointers, A5: float and double); A6 adds records.
AAPCS64_LAYOUT = {
    "int": (4, 4, "i"), "unsigned": (4, 4, "i"), "char": (1, 1, "i"), "signed char": (1, 1, "i"),
    "unsigned char": (1, 1, "i"), "short": (2, 2, "i"), "unsigned short": (2, 2, "i"),
    "int *": (8, 8, "p"), "char *": (8, 8, "p"), "float": (4, 4, "f"), "double": (8, 8, "f"),
}
TARGET = "riscv32"  # set by main; the generator draws and models by target


def round_up(v, m):
    return (v + m - 1) // m * m


def fields_of(t):
    """(name, type, count) for each field of record t."""
    return [(f[0], f[1], f[2] if len(f) > 2 else None) for f in RECORDS[t][1]]


def scalar_layout():
    """The scalar sizes of the current target: ILP32 for riscv32, LP64 (AAPCS64) for aarch64."""
    return AAPCS64_LAYOUT if TARGET == "aarch64" else SCALAR_LAYOUT


def layout(t):
    """(size, align) of a scalar or record type, by the C layout rules of the current target."""
    if t in scalar_layout():
        return scalar_layout()[t][:2]
    kind = RECORDS[t][0]
    off, align, size = 0, 1, 0
    for _, ft, n in fields_of(t):
        fs, fa = layout(ft)
        align = max(align, fa)
        off = round_up(off, fa) + fs * (n or 1)
        size = max(size, fs * (n or 1))
    return round_up(off if kind == "struct" else size, align), align


def leaves(t, prefix=""):
    """(C access path, scalar type) of every checked leaf of record t. A union
    lists only its first member, the one the caller writes."""
    out = []
    flds = fields_of(t)
    if RECORDS[t][0] == "union":
        flds = flds[:1]
    for name, ft, n in flds:
        for path in ([prefix + name] if n is None else ["%s%s[%d]" % (prefix, name, k) for k in range(n)]):
            out += leaves(ft, path + ".") if ft in RECORDS else [(path, ft)]
    return out


def decl(t):
    parts = ["%s %s%s;" % (ft, name, "" if n is None else "[%d]" % n) for name, ft, n in fields_of(t)]
    return "%s { %s };" % (t, " ".join(parts))


SCALAR_TYPES = list(INT_TYPES) + PTR_TYPES + FLOAT_TYPES
STRUCTS = STRUCT_NAMES = PARAM_TYPES = HEADER = None  # set by setup_target
# Values for float and double parameters; the integral ones can also be passed
# through an int local. Decimal text is rounded the same way by every compiler.
FEDGE = [0.0, 0.5, -1.25, 1.5, 100.0, 3.0, -7.0, 1024.5, 65536.0, 0.1, 3.141592653589793,
         2147483648.0, -2.5e-3, 1e10]
EDGE = [0, 1, -1, 0x7F, 0x80, 0xFF, 0x100, -128, -129, 0x7FFF, 0x8000, 0xFFFF,
        -32768, -32769, 65535, 65536, 0x7FFFFFFF, -2147483648, 0x80000000,
        0xFFFFFFFF, 4000000000, 0x12345678, -0x12345678, 200, -3, 60000]


def conv(v, t):
    size, signed = INT_TYPES[t]
    bits = 8 * size
    v &= (1 << bits) - 1
    if signed and v >> (bits - 1):
        v -= 1 << bits
    return v


def lit(v, t):
    """A C literal whose value (after promotion) equals v of type t."""
    if t == "unsigned":
        return "%du" % v
    if v == -2147483648:
        return "(-2147483647 - 1)"
    return "%d" % v if v >= 0 else "(%d)" % v


def flit(v, t):
    """A C floating literal of type t."""
    text = "%.17g" % v
    if "." not in text and "e" not in text and "n" not in text:
        text += ".0"
    return "(%s)" % (text + "f" if t == "float" else text) if v < 0 else (text + "f" if t == "float" else text)


def is_int_valued(v):
    return v == int(v) and abs(v) < 2 ** 31


def byte_check(k, v):
    """The test of a split double parameter: its 8 bytes, through an unsigned
    char pointer. GCC's callee spills a7 next to the caller's stack word and
    reads the double with an fld from an address that is 4 mod 8; spike pk
    emulates that misaligned fld wrongly (it returns a wrong value even for
    GCC to GCC), so a split double is never read as a double in the callee."""
    return byte_check_expr("p%d" % k, v, "%d", k)


def byte_check_expr(expr, v, sig, k):
    want = struct.pack("<d", v)
    tests = " || ".join("b[%d] != %d" % (n, c) for n, c in enumerate(want))
    return "    b = (unsigned char *)&%s;\n    if (%s) abi_fail(%s, %d);" % (expr, tests, sig, k)


def leaf_values(rng, t):
    """The values of a parameter: one number for a scalar, a list for a struct
    (a pointer leaf has no number: it is always the driver's pointer)."""
    if t in STRUCTS:
        return [rng.choice(FEDGE if lt in FLOAT_TYPES else EDGE) for _, lt in STRUCTS[t][1]]
    return rng.choice(FEDGE if t in FLOAT_TYPES else EDGE)


def leaf_literal(v, lt):
    if lt in PTR_TYPES:
        return ptr_getter(lt)
    return flit(v, lt) if lt in FLOAT_TYPES else lit(conv(v, lt), lt)


def has_double_leaf(t):
    return t in STRUCTS and any(lt == "double" for _, lt in STRUCTS[t][1])


def struct_checks(expr, t, vals, sig, k, split=False):
    """Checks every leaf of the struct 'expr'. If the struct is split between a7
    and the stack, a double leaf is compared by its bytes (see byte_check)."""
    out = []
    for (path, lt), v in zip(STRUCTS[t][1], vals):
        leaf = "%s.%s" % (expr, path)
        if lt == "double" and split:
            out.append(byte_check_expr(leaf, v, str(sig), k))
        else:
            out.append("    if (%s != %s) abi_fail(%d, %d);" % (leaf, leaf_literal(v, lt), sig, k))
    return out


def struct_assigns(expr, t, vals):
    return ["    %s.%s = %s;" % (expr, path, leaf_literal(v, lt))
            for (path, lt), v in zip(STRUCTS[t][1], vals)]


def ptr_getter(t):
    return "abi_ptr_i()" if t == "int *" else "abi_ptr_c()"


# ---- A model of the ilp32d rules (RISC-V psABI, hardware floating-point
# calling convention), independent of the compiler. It decides which ABI case
# each argument and the result hit, for the hit table, and which doubles are
# split between a7 and the stack (those are byte-checked). Whether the placement
# is right is proven by running the GCC combinations, not by this model.

def flatten(t, base=0):
    """The scalar leaves (class, offset) of t for the floating convention, or
    None: a union, a pointer, an integer over 4 bytes or more than two leaves
    disqualify (arrays and nested structs are flattened)."""
    if t in SCALAR_LAYOUT:
        cls = SCALAR_LAYOUT[t][2]
        return None if cls == "p" else [(cls, base)]
    if RECORDS[t][0] == "union":
        return None
    out, off = [], 0
    for _, ft, n in fields_of(t):
        fs, fa = layout(ft)
        off = round_up(off, fa)
        for k in range(n or 1):
            sub = flatten(ft, base + off + k * fs)
            if sub is None or len(out) + len(sub) > 2:
                return None
            out += sub
        off += fs * (n or 1)
    return out


def fp_leaves(t):
    """The leaves of a record in the floating convention (one or two, at least
    one floating), else None."""
    leaves_ = flatten(t)
    return leaves_ if leaves_ and any(c == "f" for c, _ in leaves_) else None


def has_float_member(t):
    """True if record t has a float or double member (at any depth)."""
    return any(ft in FLOAT_TYPES or (ft in RECORDS and has_float_member(ft)) for _, ft, _ in fields_of(t))


CASES = [
    ("a", "int-convention struct <= 4 bytes in one a register"),
    ("b", "5-8 byte struct in two a registers"),
    ("c", "small struct split across a7 and the stack"),
    ("d", "small struct entirely on the stack"),
    ("e", "float+float struct in two fa registers"),
    ("f", "float+int / int+float struct in fa + a"),
    ("g", "single float/double field struct in one fa"),
    ("h", "FP-eligible struct, fa or a ran out: integer convention"),
    ("i_reg", ">8-byte struct by reference, pointer in a register"),
    ("i_stack", ">8-byte struct by reference, pointer on the stack"),
    ("j", "struct return > 8 bytes via hidden pointer"),
    ("k", "small struct return in a0/a1"),
    ("l", "FP struct return in fa0/fa1"),
    ("m", "union containing floats (integer convention)"),
    ("n", "struct argument after fa0-fa7 are used up"),
    ("x_dsplit", "(scalar) double split across a7 and the stack"),
    ("x_pad8", "8-aligned value on the stack after a padding word"),
]


def classify(params, ret):
    """(hits, split): the ABI cases each argument and the result hit (a list,
    one entry per occurrence), and the indexes of the parameters (doubles or
    records) that are split between a7 and the stack."""
    hits, split = [], set()
    a = fa = stack = 0

    def on_stack(t):  # aligned to its type (at least 4), padded to 4 bytes
        nonlocal stack
        size, align = layout(t)
        if stack % max(align, 4):
            hits.append("x_pad8")
        stack = round_up(stack, max(align, 4)) + round_up(size, 4)

    if ret in RECORDS:
        hits += ["m"] if RECORDS[ret][0] == "union" and has_float_member(ret) else []
        if fp_leaves(ret):
            hits.append("l")
        elif layout(ret)[0] > 8:
            hits.append("j")
            a = 1  # the hidden result pointer takes a0
        else:
            hits.append("k")
    for k, t in enumerate(params):
        if t in RECORDS:
            size = layout(t)[0]
            hits += ["n"] if fa == 8 else []
            hits += ["m"] if RECORDS[t][0] == "union" and has_float_member(t) else []
            fl = fp_leaves(t)
            if fl:
                nf = sum(c == "f" for c, _ in fl)
                ni = len(fl) - nf
                if fa + nf <= 8 and a + ni <= 8:
                    fa, a = fa + nf, a + ni
                    hits.append("f" if ni else ("e" if nf == 2 else "g"))
                    continue
                hits.append("h")
            if size > 8:
                hits.append("i_reg" if a < 8 else "i_stack")
                if a == 8:
                    on_stack("int *")
                a = min(a + 1, 8)
            elif a == 8:
                hits.append("d")
                on_stack(t)
            elif size <= 4:
                hits.append("a")
                a += 1
            elif a <= 6:
                hits.append("b")
                a += 2
            else:
                hits.append("c")
                split.add(k)
                a = 8
                stack += 4
        elif t in FLOAT_TYPES and fa < 8:
            fa += 1
        elif t == "double" and a == 7:
            hits.append("x_dsplit")
            split.add(k)
            a = 8
            stack += 4
        elif a < 8:
            a = min(a + (2 if t == "double" else 1), 8)
        else:
            on_stack(t)
    return hits, split


# ---- A model of AAPCS64 (Procedure Call Standard for the Arm 64-bit Architecture),
# section 6.8.2 "Parameter passing" (stages A, B, C), 6.8.3 "Homogeneous Aggregates" and
# 6.9 "Result return", written from the standard's text and independent of the compiler's
# classifier. Rule numbers are those of the current ARM-software/abi-aa aapcs64.rst (checked
# 2026-11-19). Each rule below carries its number and a paraphrase of the standard's wording.

AAPCS64_CASES = [
    ("x_int", "4-byte integer in an x register (C.9)"),
    ("x_ptr", "pointer in an x register (C.9)"),
    ("x_narrow", "char/short in an x register (C.9; upper bits unspecified)"),
    ("x_last", "the eighth integer or pointer argument, in x7"),
    ("x_after_v", "integer or pointer in an x register once v0-v7 are used up (NSRN 8, NGRN < 8)"),
    ("v_float", "float in an s register (C.1)"),
    ("v_double", "double in a d register (C.1)"),
    ("v_last", "the eighth float or double argument, in v7 (C.1)"),
    ("v_after_x", "float or double in a v register once x0-x7 are used up (NGRN 8, NSRN < 8)"),
    ("s_first", "the first stack argument (NSAA 0) (C.6 or C.14, C.17)"),
    ("s_int", "4-byte integer on the stack (C.14, C.17)"),
    ("s_ptr", "pointer on the stack (C.14, C.17)"),
    ("s_narrow", "char/short on the stack, 8-byte slot (C.16)"),
    ("s_float", "float on the stack, 8-byte slot (C.5, C.6)"),
    ("s_double", "double on the stack (C.6)"),
    ("s_int_vfree", "integer or pointer on the stack while v registers are free (NGRN 8, NSRN < 8)"),
    ("s_fp_xfree", "float or double on the stack while x registers are free (NSRN 8, NGRN < 8)"),
    ("s_mixed", "a stack argument of the other class than the one before it (one NSAA for both)"),
    ("s_next", "a stack argument at NSAA > 0 (slots advance by 8)"),
    ("s_odd", "stack area not a multiple of 16 before padding (SP alignment)"),
    ("r_int", "4-byte integer result in w0"),
    ("r_ptr", "pointer result in x0"),
    ("r_narrow", "char/short result in w0 (upper bits unspecified)"),
    ("r_float", "float result in s0 (6.9: where void f(T) gets its argument)"),
    ("r_double", "double result in d0 (6.9)"),
    # HFAs in v registers (C.2)
    ("h_v1", "HFA of 1 member in v registers (C.2)"),
    ("h_v2", "HFA of 2 members in v registers (C.2)"),
    ("h_v3", "HFA of 3 members in v registers (C.2)"),
    ("h_v4", "HFA of 4 members in v registers (C.2)"),
    ("h_float", "HFA of floats in v registers"),
    ("h_double", "HFA of doubles in v registers"),
    ("h_nested", "HFA built with a nested record, in v registers"),
    ("h_array", "HFA built with an array, in v registers"),
    ("h_union", "HFA that is or holds a union, in v registers"),
    ("h_big_v", "HFA over 16 bytes by value in v registers (B.3 before B.4)"),
    ("h_last", "HFA ending in v7 (NSRN + members = 8)"),
    # HFAs on the stack (C.3, C.4, C.6)
    ("h_stack", "HFA on the stack, v registers used up before it (NSRN 8)"),
    ("h_stack_part", "HFA on the stack while NSRN < 8 but too few v registers left (C.3 sets NSRN 8)"),
    ("h_stack_fp", "float or double on the stack after C.3 set NSRN 8, though v[NSRN] was free before"),
    ("h_big_stack", "HFA over 16 bytes on the stack by value"),
    # records in x registers (C.12)
    ("c_x1", "non-HFA record of up to 8 bytes in one x register (C.12)"),
    ("c_x2", "non-HFA record of 9 to 16 bytes in two x registers (C.12)"),
    ("c_x_odd", "record of a size that is not 1, 2, 4, 8 or 16, in x registers"),
    ("c_x_float", "non-HFA record holding a float or double, in x registers"),
    ("c_x_union", "union in x registers"),
    ("c_x_last", "record ending in x7"),
    # records on the stack (C.13, C.15)
    ("c_stack_one", "9-16 byte record on the stack with one x register left (NGRN 7: C.13, no split)"),
    ("c_stack_int", "integer or pointer on the stack after C.13 left x7 unused"),
    ("c_stack_full", "record on the stack once x0-x7 are used up (NGRN 8)"),
    ("c_stack_odd", "odd-sized record on the stack followed by another stack argument (B.5 rounding)"),
    # by reference (B.4)
    ("ref_x", "record over 16 bytes (not an HFA) by reference, pointer in an x register"),
    ("ref_stack", "record over 16 bytes (not an HFA) by reference, pointer on the stack"),
    # results (6.9, derived from stage C)
    ("r_rec_x1", "record result of up to 8 bytes in x0"),
    ("r_rec_x2", "record result of 9 to 16 bytes in x0/x1"),
    ("r_hfa", "HFA result in v registers"),
    ("r_hfa4", "4-member HFA result in v0-v3"),
    ("r_mem", "record result in memory, address in x8"),
    ("r_mem_xfull", "x8 result while x0-x7 are all used by arguments"),
]


def aapcs64_kind(t):
    """'int', 'ptr', 'narrow', 'float' or 'double' for a scalar type of the model."""
    size, _, cls = AAPCS64_LAYOUT[t]
    if cls == "f":
        return "float" if size == 4 else "double"
    return "ptr" if cls == "p" else ("narrow" if size < 4 else "int")


def fdt_members(t):
    """(set of Fundamental Data Types, number of uniquely addressable members) of type t after layout.
    The members of a struct are all there is in it, so they add up; the members of a union overlap, so
    only the largest member counts (the standard's size of an HFA is the base type's size times the
    number of members, and a union's size is that of its largest member). An array of n elements has
    n times the members of its element type."""
    if t not in RECORDS:
        return {t}, 1
    parts = [(fdt_members(ft), n or 1) for _, ft, n in fields_of(t)]
    kinds = set().union(*(k for (k, _), _ in parts))
    counts = [c * n for (_, c), n in parts]
    return kinds, (max(counts) if RECORDS[t][0] == "union" else sum(counts))


def hfa_of(t):
    """(base type, members) if composite type t is a Homogeneous Floating-point Aggregate: every Fundamental
    Data Type in it is the same floating-point type, there are 1 to 4 uniquely addressable members (and no
    padding: the size is the members times the base size, which the union rule above makes true). Else None."""
    if t not in RECORDS:
        return None
    kinds, count = fdt_members(t)
    if len(kinds) != 1 or not kinds <= set(FLOAT_TYPES) or not 1 <= count <= 4:
        return None
    base = next(iter(kinds))
    return (base, count) if layout(t)[0] == count * layout(base)[0] else None


class Aapcs64State:
    """Stage A: A.1 NGRN = 0, A.2 NSRN = 0, A.4 NSAA = SP (kept as a byte offset from SP). A.3 (NPRN, scalable
    predicates) does not exist for our types. The two holes record what C.3 and C.13 skipped, for the hit table."""

    def __init__(self):
        self.ngrn = self.nsrn = self.nsaa = 0
        self.v_hole = self.x_hole = None  # NSRN / NGRN before C.3 / C.13 set it to 8


class Aapcs64Arg:
    """The argument as stage B and the C rules see it: size, natural alignment and what it is."""

    def __init__(self, t):
        self.size, self.align = layout(t)
        self.composite = t in RECORDS
        self.hfa = hfa_of(t)
        self.floating = not self.composite and AAPCS64_LAYOUT[t][2] == "f"
        self.by_ref = False


Place = collections.namedtuple("Place", "where regs offset by_ref size")  # where: 'x', 'v' or 'stack'


def aapcs64_stage_b(a):
    """Stage B, the first matching rule. B.1 (Pure Scalable Types), B.2 (a composite whose size the
    callee cannot know) and B.6 (alignment adjusted types) cannot apply to the types of this model."""
    if a.hfa:                      # B.3: an HFA is used unmodified
        return
    if a.composite and a.size > 16:  # B.4: copied to memory, the argument becomes a pointer to the copy
        a.size, a.align, a.composite, a.by_ref = 8, 8, False, True
    elif a.composite:              # B.5: the size is rounded up to a multiple of 8
        a.size = round_up(a.size, 8)


def stack_place(st, a):
    off = st.nsaa
    st.nsaa += a.size
    return Place("stack", [], off, a.by_ref, a.size)


# Stage C: each rule takes (state, argument), changes them as the standard says, and returns the Place
# if "the argument has now been allocated", else None (the next rule applies).

def c1(st, a):  # a floating-point argument goes in v[NSRN] if NSRN < 8; NSRN += 1
    if a.floating and st.nsrn < 8:
        st.nsrn += 1
        return Place("v", [st.nsrn - 1], 0, False, a.size)


def c2(st, a):  # an HFA goes in one v register per member if NSRN + members <= 8; NSRN += members
    if a.hfa and st.nsrn + a.hfa[1] <= 8:
        regs = list(range(st.nsrn, st.nsrn + a.hfa[1]))
        st.nsrn += a.hfa[1]
        return Place("v", regs, 0, False, a.size)


def c3(st, a):  # an HFA that did not fit: NSRN = 8 and its size is rounded up to a multiple of 8
    if a.hfa:
        st.v_hole = st.nsrn if st.nsrn < 8 else st.v_hole
        st.nsrn = 8
        a.size = round_up(a.size, 8)


def c4(st, a):  # an HFA: NSAA is rounded up to the next multiple of 8 (natural alignment <= 8)
    if a.hfa:
        st.nsaa = round_up(st.nsaa, 8)


def c5(st, a):  # a single-precision argument's size is set to 8 bytes
    if a.floating and a.size == 4:
        a.size = 8


def c6(st, a):  # an HFA or a floating-point argument is copied to memory at NSAA; NSAA += size
    if a.hfa or a.floating:
        return stack_place(st, a)


def c7(st, a):  # Pure Scalable Types: none among our types
    assert not hasattr(a, "scalable")


def c8(st, a):  # a Pure Scalable Type not allocated by C.7: none
    assert not hasattr(a, "scalable")


def c9(st, a):  # an integral or pointer argument of at most 8 bytes goes in x[NGRN] if NGRN < 8; NGRN += 1
    if not a.composite and not a.floating and a.size <= 8 and st.ngrn < 8:
        st.ngrn += 1
        return Place("x", [st.ngrn - 1], 0, a.by_ref, a.size)


def c10(st, a):  # an argument of alignment 16 rounds NGRN up to even: no 16-byte aligned type here
    assert a.align != 16


def c11(st, a):  # a 16-byte integral argument goes in two x registers: no __int128 here
    assert a.composite or a.floating or a.size <= 8


def c12(st, a):  # a composite of at most 8 - NGRN double-words goes in consecutive x registers
    if a.composite and a.size // 8 <= 8 - st.ngrn:
        regs = list(range(st.ngrn, st.ngrn + a.size // 8))
        st.ngrn += len(regs)
        return Place("x", regs, 0, False, a.size)


def c13(st, a):  # NGRN = 8: what does not fit is never split between registers and the stack
    st.x_hole = st.ngrn if st.ngrn < 8 else st.x_hole
    st.ngrn = 8


def c14(st, a):  # NSAA is rounded up to a multiple of max(8, natural alignment)
    st.nsaa = round_up(st.nsaa, max(8, a.align))


def c15(st, a):  # a composite is copied to memory at NSAA; NSAA += size
    if a.composite:
        return stack_place(st, a)


def c16(st, a):  # a size under 8 bytes is set to 8
    a.size = max(a.size, 8)


def c17(st, a):  # the argument is copied to memory at NSAA; NSAA += size
    return stack_place(st, a)


AAPCS64_STAGE_C = [c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17]


def aapcs64_place(st, t):
    """Stages B and C for one argument of scalar or record type t, in the standard's order. Returns the
    Place (the v or x registers, or the byte offset from SP, and whether it was replaced by a pointer
    to a copy by B.4) and advances st."""
    a = Aapcs64Arg(t)
    aapcs64_stage_b(a)
    for rule in AAPCS64_STAGE_C:
        place = rule(st, a)
        if place:
            return place
    raise AssertionError("C.17 allocates every argument")


def aapcs64_result(t):
    """6.9: the result of type T is where the argument of void f(T) would be passed as a value in registers
    (stage A, B, C on a fresh state); otherwise (it would be on the stack, or replaced by a pointer: B.4) it is
    in caller-reserved memory whose address is passed in x8. Returns ('x', regs), ('v', regs) or ('mem', [8])."""
    place = aapcs64_place(Aapcs64State(), t)
    if place.where == "stack" or place.by_ref:
        return ("mem", [8])
    return (place.where, place.regs)


def has_member(t, test):
    """True if record t has, at any depth, a field for which test(type, array count) holds."""
    return any(test(ft, n) or (ft in RECORDS and has_member(ft, test)) for _, ft, n in fields_of(t))


def has_union(t):
    return RECORDS[t][0] == "union" or has_member(t, lambda ft, n: ft in RECORDS and RECORDS[ft][0] == "union")


def record_hits(t, place, before, st):
    """The cases of a record argument. before is (NGRN, NSRN) when the argument came, st the state after it
    (st.x_hole and st.v_hole are the holes C.13 and C.3 left, for the later arguments)."""
    ngrn, nsrn = before
    size = layout(t)[0]
    if place.by_ref:
        return ["ref_x" if place.where == "x" else "ref_stack"]
    hits = []
    if place.where == "v":
        base, n = hfa_of(t)
        hits += ["h_v%d" % n, "h_" + base]
        hits += ["h_nested"] if has_member(t, lambda ft, c: ft in RECORDS) else []
        hits += ["h_array"] if has_member(t, lambda ft, c: c is not None) else []
        hits += ["h_union"] if has_union(t) else []
        hits += ["h_big_v"] if size > 16 else []
        hits += ["h_last"] if place.regs[-1] == 7 else []
    elif place.where == "x":
        hits.append("c_x1" if len(place.regs) == 1 else "c_x2")
        hits += ["c_x_odd"] if size not in (1, 2, 4, 8, 16) else []
        hits += ["c_x_float"] if has_float_member(t) else []
        hits += ["c_x_union"] if RECORDS[t][0] == "union" else []
        hits += ["c_x_last"] if place.regs[-1] == 7 else []
    elif hfa_of(t):
        hits += ["h_stack" if nsrn == 8 else "h_stack_part"]
        hits += ["h_big_stack"] if size > 16 else []
    else:
        hits += ["c_stack_one"] if ngrn == 7 and size > 8 else []
        hits += ["c_stack_full"] if ngrn == 8 else []
    return hits


def classify_aapcs64(params, ret):
    """(hits, split) like classify: the AAPCS64 cases each argument and the result hit
    (one entry per occurrence). Nothing is split between registers and the stack on
    aarch64, so 'split' is always empty. A type the model does not know raises KeyError."""
    hits = []
    st = Aapcs64State()
    stacked = []        # for each stack argument: its class, 'f' (floating), 'i' (integer, pointer) or 'r' (record)
    odd_pending = False  # an odd-sized record is on the stack and no stack argument followed yet
    for t in params:
        is_rec = t in RECORDS
        cls = "r" if is_rec else ("f" if AAPCS64_LAYOUT[t][2] == "f" else "i")
        before = (st.ngrn, st.nsrn)
        place = aapcs64_place(st, t)
        if is_rec:
            hits += record_hits(t, place, before, st)
        elif place.where == "x":
            hits += ["x_" + aapcs64_kind(t)] + (["x_last"] if place.regs[0] == 7 else []) + (["x_after_v"] if before[1] == 8 else [])
        elif place.where == "v":
            hits += ["v_" + aapcs64_kind(t)] + (["v_last"] if place.regs[0] == 7 else []) + (["v_after_x"] if before[0] == 8 else [])
        if place.where == "stack":
            hits.append("s_first" if place.offset == 0 else "s_next")
            if not is_rec:
                hits.append("s_" + aapcs64_kind(t))
                hits += ["s_fp_xfree"] if cls == "f" and before[0] < 8 else []
                hits += ["s_int_vfree"] if cls == "i" and before[1] < 8 else []
                hits += ["h_stack_fp"] if cls == "f" and st.v_hole is not None else []
                hits += ["c_stack_int"] if cls == "i" and st.x_hole is not None else []
            hits += ["s_mixed"] if stacked and stacked[-1] != cls else []
            hits += ["c_stack_odd"] if odd_pending else []
            odd_pending = is_rec and layout(t)[0] % 8 != 0 and not place.by_ref and not hfa_of(t)
            stacked.append(cls)
    hits += ["s_odd"] if st.nsaa % 16 else []
    if ret != "void":
        where, regs = aapcs64_result(ret)
        if ret in RECORDS:
            n = hfa_of(ret)
            hits.append("r_mem" if where == "mem" else ("r_hfa" if where == "v" else
                        ("r_rec_x1" if len(regs) == 1 else "r_rec_x2")))
            hits += ["r_hfa4"] if n and n[1] == 4 else []
            hits += ["r_mem_xfull"] if where == "mem" and st.ngrn == 8 else []
        else:
            hits.append("r_" + aapcs64_kind(ret))
    return hits, set()


def a_users(rng, n):
    """n parameters that each take one a register (integers and pointers)."""
    return [rng.choice(list(INT_TYPES) + PTR_TYPES) for _ in range(n)]


def gen_params(rng, ret):
    """A third of the signatures are uniform draws (0 to 14 parameters); the
    others are shaped, since uniform draws rarely reach the edges:
    'fp_heavy' has 9 to 12 float/double parameters among 0 to 8 others (floats
    and doubles in a registers, doubles in pairs and on the stack); 'split' has
    8 float/double and 7 other parameters in any order, then a double (low
    word in a7, high word on the stack), then 0 to 3 more parameters.
    'structs' has 1 to 6 structs or unions among 0 to 9 scalars. 'fa_full'
    uses up fa0-fa7 with doubles (and some floats), then passes structs, which
    must fall back to the integer convention. 'struct_split' takes a0-a6 (a1-a6
    if the result has a hidden pointer) with integers, among at most 4 floats,
    then passes a 5 to 8 byte struct (its low word goes in a7, its high word on
    the stack); half the time fa0-fa7 are used up first, so a floating struct
    is split too. 'a_full' takes a0-a7 (and sometimes the first stack word),
    then passes structs (on the stack, by a pointer on the stack, or in fa only)."""
    shape = rng.choice(["random", "random", "random", "fp_heavy", "split", "structs", "structs",
                        "fa_full", "struct_split", "struct_split", "a_full"])
    others = list(INT_TYPES) + PTR_TYPES
    hidden = ret in RECORDS and not fp_leaves(ret) and layout(ret)[0] > 8
    if shape == "random":
        return [rng.choice(PARAM_TYPES) for _ in range(rng.randint(0, 14))]
    if shape == "structs":
        params = [rng.choice(STRUCT_NAMES) for _ in range(rng.randint(1, 6))]
        params += [rng.choice(SCALAR_TYPES) for _ in range(rng.randint(0, 9))]
        rng.shuffle(params)
        return params
    if shape == "fa_full":
        params = [rng.choice(["double", "double", "float"]) for _ in range(8)] + a_users(rng, rng.randint(0, 3))
        rng.shuffle(params)
        tail = [rng.choice(STRUCT_NAMES) for _ in range(rng.randint(1, 4))]
        tail += [rng.choice(PARAM_TYPES) for _ in range(rng.randint(0, 3))]
        rng.shuffle(tail)
        return params + tail
    if shape == "struct_split":
        fa_full = rng.random() < 0.5
        params = a_users(rng, 6 if hidden else 7)
        params += [rng.choice(FLOAT_TYPES) for _ in range(8 if fa_full else rng.randint(0, 4))]
        rng.shuffle(params)
        mid = [t for t in STRUCT_NAMES if 4 < layout(t)[0] <= 8 and (fa_full or not fp_leaves(t))]
        return params + [rng.choice(mid)] + [rng.choice(PARAM_TYPES) for _ in range(rng.randint(0, 3))]
    if shape == "a_full":
        # 8 or 9: a ninth integer puts the next 8-aligned struct after a padding word
        params = a_users(rng, rng.choice([8, 9])) + [rng.choice(FLOAT_TYPES) for _ in range(rng.randint(0, 4))]
        rng.shuffle(params)
        return params + [rng.choice(STRUCT_NAMES) for _ in range(rng.randint(1, 5))] + \
            [rng.choice(SCALAR_TYPES) for _ in range(rng.randint(0, 2))]
    if shape == "fp_heavy":
        params = [rng.choice(FLOAT_TYPES) for _ in range(rng.randint(9, 12))]
        params += [rng.choice(others) for _ in range(rng.randint(0, 8))]
        rng.shuffle(params)
        return params
    params = [rng.choice(FLOAT_TYPES) for _ in range(8)] + [rng.choice(others) for _ in range(7)]
    rng.shuffle(params)
    return params + ["double"] + [rng.choice(SCALAR_TYPES) for _ in range(rng.randint(0, 3))]


AARCH64_SHAPES = ["random", "random", "stack", "stack", "x_full", "narrow_tail", "ptr_tail",
                  "fp_heavy", "v_full", "both_full",
                  "records", "records", "records", "hfa_tail", "hfa_tail", "x7_left", "x7_left",
                  "x_full_rec", "x_full_rec", "x8_full"]


def hfa_records(min_members=1):
    return [t for t in STRUCT_NAMES if hfa_of(t) and hfa_of(t)[1] >= min_members]


def x_records(lo, hi):
    """Records that go in x registers: not an HFA, from lo to hi bytes."""
    return [t for t in STRUCT_NAMES if not hfa_of(t) and lo <= layout(t)[0] <= hi]


def gen_params_aarch64(rng, shape):
    """Parameter lists that reach x0-x7, v0-v7 and the stack, by shape. Scalar shapes: uniform draws
    (0 to 14 parameters) and shapes that always use the stack: 'stack' (9 to 20 parameters), 'x_full' (8
    integers or pointers, then 1 to 4 of any type, so floats go in v registers after x0-x7), 'narrow_tail'
    (8 integers or pointers, then 1 to 8 char/short types, so narrow values sit in stack slots), 'ptr_tail'
    (8 integers, then pointers), 'fp_heavy' (9 to 14 floats and doubles among 0 to 6 integers: floating
    values on the stack while x registers are free), 'v_full' (8 floats and doubles, then 1 to 4 of any
    type: integers in x registers after v0-v7) and 'both_full' (8 of each class in any order, then 1 to 6
    of any type: both classes interleaved on the stack). Record shapes: 'records' (1 to 6 records among 0 to
    9 scalars), 'hfa_tail' (4 to 8 floats and doubles, then HFAs of more members than v registers are left,
    then floats and doubles that would have fitted without C.3, then records), 'x7_left' (7 integers or
    pointers, then a 9 to 16 byte record that is not an HFA: NGRN 7, C.13; then integers, pointers and
    records), 'x_full_rec' (8 integers or pointers, then 1 to 4 records and maybe floats: records on the
    stack, by reference with the pointer on the stack, HFAs in v registers) and 'x8_full' (8 integers or
    pointers and 0 to 2 records; the result, drawn by gen_sig_aarch64, is in memory through x8)."""
    ints, narrow = list(INT_TYPES), ["char", "signed char", "unsigned char", "short", "unsigned short"]
    anyt = ints + PTR_TYPES
    allt = anyt + FLOAT_TYPES
    recs = STRUCT_NAMES
    if shape == "random":
        return [rng.choice(allt) for _ in range(rng.randint(0, 14))]
    if shape == "stack":
        return [rng.choice(allt) for _ in range(rng.randint(9, 20))]
    if shape == "x_full":
        return [rng.choice(anyt) for _ in range(8)] + [rng.choice(allt) for _ in range(rng.randint(1, 4))]
    if shape == "narrow_tail":
        return [rng.choice(anyt) for _ in range(8)] + [rng.choice(narrow) for _ in range(rng.randint(1, 8))]
    if shape == "ptr_tail":
        return [rng.choice(ints) for _ in range(8)] + [rng.choice(PTR_TYPES) for _ in range(rng.randint(1, 4))]
    if shape == "fp_heavy":
        params = [rng.choice(FLOAT_TYPES) for _ in range(rng.randint(9, 14))]
        params += [rng.choice(anyt) for _ in range(rng.randint(0, 6))]
        rng.shuffle(params)
        return params
    if shape == "v_full":
        return [rng.choice(FLOAT_TYPES) for _ in range(8)] + [rng.choice(allt) for _ in range(rng.randint(1, 4))]
    if shape == "records":
        params = [rng.choice(recs) for _ in range(rng.randint(1, 6))]
        params += [rng.choice(allt) for _ in range(rng.randint(0, 9))]
        rng.shuffle(params)
        return params
    if shape == "hfa_tail":
        k = rng.randint(4, 8)
        head = [rng.choice(FLOAT_TYPES) for _ in range(k)]
        too_big = [t for t in hfa_records(2) if hfa_of(t)[1] > 8 - k] or hfa_records()
        mid = [rng.choice(too_big) for _ in range(rng.randint(1, 2))]
        tail = [rng.choice(FLOAT_TYPES) for _ in range(rng.randint(1, 3))]
        tail += [rng.choice(recs) for _ in range(rng.randint(0, 2))]
        rng.shuffle(tail)
        return head + mid + tail
    if shape == "x7_left":
        head = [rng.choice(anyt) for _ in range(7)]
        mid = [rng.choice(x_records(9, 16))]
        tail = [rng.choice(anyt) for _ in range(rng.randint(1, 3))]
        tail += [rng.choice(recs) for _ in range(rng.randint(0, 2))] + [rng.choice(FLOAT_TYPES) for _ in range(rng.randint(0, 2))]
        rng.shuffle(tail)
        return head + mid + tail
    if shape == "x_full_rec":
        tail = [rng.choice(recs) for _ in range(rng.randint(1, 4))] + [rng.choice(FLOAT_TYPES) for _ in range(rng.randint(0, 2))]
        rng.shuffle(tail)
        return [rng.choice(anyt) for _ in range(8)] + tail
    if shape == "x8_full":
        tail = [rng.choice(recs) for _ in range(rng.randint(0, 2))]
        return [rng.choice(anyt) for _ in range(8)] + tail
    params = [rng.choice(anyt) for _ in range(8)] + [rng.choice(FLOAT_TYPES) for _ in range(8)]
    rng.shuffle(params)
    return params + [rng.choice(allt) for _ in range(rng.randint(1, 6))]


def memory_records():
    """Records whose result is in memory (through x8): not an HFA, over 16 bytes."""
    return [t for t in STRUCT_NAMES if not hfa_of(t) and layout(t)[0] > 16]


def gen_sig_aarch64(rng):
    shape = rng.choice(AARCH64_SHAPES)
    if shape == "x8_full":
        ret = rng.choice(memory_records())
    elif rng.random() < 0.5:
        ret = rng.choice(STRUCT_NAMES)
    else:
        ret = rng.choice(list(INT_TYPES) + PTR_TYPES + FLOAT_TYPES + ["void"])
    params = gen_params_aarch64(rng, shape)
    raws = [leaf_values(rng, t) for t in params]
    modes = [rng.choice(["lit", "same", "int", "call"]) for _ in params]
    if ret in STRUCTS:  # the values of the record the callee builds and returns
        raws.append(leaf_values(rng, ret))
    return params, ret, raws, modes


def split_params(params, ret="void"):
    """Indexes of the parameters (doubles, structs and unions) that ilp32d splits
    between a7 and the stack (see classify)."""
    return classify_for_target(params, ret)[1]


def classify_for_target(params, ret):
    return classify_aapcs64(params, ret) if TARGET == "aarch64" else classify(params, ret)


def gen_sig(rng):
    if TARGET == "aarch64":
        return gen_sig_aarch64(rng)
    ret = rng.choice(PARAM_TYPES + ["void"])
    params = gen_params(rng, ret)
    raws = [leaf_values(rng, t) for t in params]
    modes = [rng.choice(["lit", "same", "int", "call"]) for _ in params]
    if ret in STRUCTS:  # the values of the struct the callee builds and returns
        raws.append(leaf_values(rng, ret))
    return params, ret, raws, modes


def expected_return(params, ret, raws):
    acc = 0
    for t, r in zip(params, raws):
        if t in INT_TYPES:
            acc = (acc * 31 + (conv(r, t) & 0xFFFFFFFF)) & 0xFFFFFFFF
    return conv(acc, ret) if ret in INT_TYPES else None


def proto(i, params, ret):
    plist = ", ".join("%s p%d" % (t, k) for k, t in enumerate(params)) or "void"
    return "%s f%d(%s)" % (ret, i, plist)


def returned_param(params, ret):
    """The parameter a float or pointer return gives back: the last one of the
    return type that is not a split double (see byte_check), or None."""
    split = split_params(params, ret)
    same = [k for k, t in enumerate(params) if t == ret and k not in split]
    return same[-1] if same else None


def callee(i, params, ret, raws):
    out = [proto(i, params, ret), "{"]
    split = split_params(params, ret)
    if any(k in split and (t == "double" or has_double_leaf(t)) for k, t in enumerate(params)):
        out.append("    unsigned char *b;")
    if ret in STRUCTS:
        out.append("    %s r;" % ret)
    if ret in INT_TYPES:
        out.append("    unsigned acc;")
        out.append("    acc = 0;")
    for k, (t, r) in enumerate(zip(params, raws)):
        if t in STRUCTS:
            out += struct_checks("p%d" % k, t, r, i, k, k in split)
        elif k in split:
            out.append(byte_check(k, r) % i)
        elif t in INT_TYPES:
            out.append("    if (p%d != %s) abi_fail(%d, %d);" % (k, lit(conv(r, t), t), i, k))
        elif t in FLOAT_TYPES:
            out.append("    if (p%d != %s) abi_fail(%d, %d);" % (k, flit(r, t), i, k))
        else:
            out.append("    if (p%d != %s) abi_fail(%d, %d);" % (k, ptr_getter(t), i, k))
    if ret in INT_TYPES:
        for k, t in enumerate(params):
            if t in INT_TYPES:
                out.append("    acc = acc * 31u + p%d;" % k)
        out.append("    return acc;")
    elif ret in STRUCTS:
        out += struct_assigns("r", ret, raws[len(params)])
        out.append("    return r;")
    elif ret in PTR_TYPES:
        same = [k for k, t in enumerate(params) if t == ret]
        out.append("    return %s;" % ("p%d" % same[-1] if same else ptr_getter(ret)))
    elif ret in FLOAT_TYPES:
        k = returned_param(params, ret)
        out.append("    return %s;" % ("p%d" % k if k is not None else flit(2.5, ret)))
    out.append("}")
    return "\n".join(out)
def caller(i, params, ret, raws, modes):
    out = ["int caller%d(void)" % i, "{"]
    args = []
    decls, inits = [], []
    for k, (t, r, m) in enumerate(zip(params, raws, modes)):
        if t in STRUCTS:
            decls.append("    %s a%d;" % (t, k))
            inits += struct_assigns("a%d" % k, t, r)
            args.append("a%d" % k)
        elif t in FLOAT_TYPES:
            if m == "lit":
                args.append(flit(r, t))
            elif m in ("int", "call") and is_int_valued(r):
                decls.append("    int a%d;" % k)
                inits.append("    a%d = %d;" % (k, int(r)))
                args.append("a%d" % k)  # converted to the parameter type at the call
            else:
                decls.append("    %s a%d;" % (t, k))
                inits.append("    a%d = %s;" % (k, flit(r, t)))
                args.append("a%d" % k)
        elif t in PTR_TYPES:
            decls.append("    %s a%d;" % (t, k))
            inits.append("    a%d = %s;" % (k, ptr_getter(t)))
            args.append("a%d" % k)
        elif m == "lit":
            args.append(lit(conv(r, t), t))
        elif m == "same":
            decls.append("    %s a%d;" % (t, k))
            inits.append("    a%d = %s;" % (k, lit(conv(r, t), t)))
            args.append("a%d" % k)
        elif m == "int":
            decls.append("    int a%d;" % k)
            inits.append("    a%d = %s;" % (k, lit(conv(r, "int"), "int")))
            args.append("a%d" % k)
        else:  # an opaque int from the driver, converted at the call
            args.append("abi_int(%s)" % lit(conv(r, "int"), "int"))
    if ret != "void":
        decls.append("    %s r;" % ret)
    out += decls + inits
    call = "f%d(%s)" % (i, ", ".join(args))
    if ret == "void":
        out.append("    %s;" % call)
    else:
        out.append("    r = %s;" % call)
        if ret in INT_TYPES:
            e = expected_return(params, ret, raws)
            out.append("    if (r != %s) abi_fail(%d, 99);" % (lit(e, ret), i))
        elif ret in STRUCTS:
            out += struct_checks("r", ret, raws[len(params)], i, 99)
        elif ret in FLOAT_TYPES:
            k = returned_param(params, ret)
            want = flit(raws[k], ret) if k is not None else flit(2.5, ret)
            out.append("    if (r != %s) abi_fail(%d, 99);" % (want, i))
        else:
            out.append("    if (r != %s) abi_fail(%d, 99);" % (ptr_getter(ret), i))
    out.append("    return 0;")
    out.append("}")
    return "\n".join(out)


HEADER_FUNCS = "void abi_fail(int sig, int param);\nchar *abi_ptr_c(void);\nint *abi_ptr_i(void);\nint abi_int(int x);\n"


def setup_target(target):
    """Selects the target: its scalar layout and its record table (aarch64 also draws AARCH64_RECORDS),
    and derives everything that comes from the table (declarations, leaves, type lists, C header)."""
    global TARGET, STRUCTS, STRUCT_NAMES, PARAM_TYPES, HEADER
    TARGET = target
    RECORDS.clear()
    RECORDS.update(COMMON_RECORDS)
    if target == "aarch64":
        RECORDS.update(AARCH64_RECORDS)
    STRUCTS = {t: (decl(t), leaves(t)) for t in RECORDS}
    STRUCT_NAMES = list(STRUCTS)
    PARAM_TYPES = SCALAR_TYPES + STRUCT_NAMES
    HEADER = HEADER_FUNCS + "\n".join(d for d, _ in STRUCTS.values()) + "\n"


setup_target("riscv32")

DRIVER = r"""#include <stdio.h>
static int failures;
static char cbuf[8];
static int ibuf[2];
void abi_fail(int sig, int param) {
    failures++;
    if (param == 99) printf("FAIL sig %d return\n", sig);
    else printf("FAIL sig %d param %d\n", sig, param);
}
char *abi_ptr_c(void) { return cbuf + 3; }
int *abi_ptr_i(void) { return ibuf + 1; }
int abi_int(int x) { return x; }
%DECLS%
int main(void) {
%CALLS%
    printf("abi: %d signatures, %d failures\n", %N%, failures);
    return failures != 0;
}
"""


def hit_table(seed, sigs):
    """How many signatures (and arguments or results) hit each ABI case, by the
    model in classify. A case with no hits is not tested by this run."""
    occurrences, signatures = collections.Counter(), collections.Counter()
    cases = AAPCS64_CASES if TARGET == "aarch64" else CASES
    for params, ret, _, _ in sigs:
        hits = classify_for_target(params, ret)[0]
        occurrences.update(hits)
        signatures.update(set(hits))
    lines = ["hits seed %d (signatures / occurrences):" % seed]
    width = max(8, max(len(key) for key, _ in cases))  # 8 keeps the ilp32d table as it was
    lines += ["  %-*s %4d / %4d  %s" % (width, key, signatures[key], occurrences[key], text) for key, text in cases]
    missing = [key for key, _ in cases if not occurrences[key]]
    lines.append("  NOT HIT: " + " ".join(missing) if missing else "  every case hit")
    return "\n".join(lines)


def main():
    seed, count, out = int(sys.argv[1]), int(sys.argv[2]), sys.argv[3]
    target = sys.argv[4] if len(sys.argv) > 4 else "riscv32"
    if target not in ("riscv32", "aarch64"):
        sys.exit("unknown target " + target)
    setup_target(target)
    rng = random.Random(seed)
    sigs = [gen_sig(rng) for _ in range(count)]
    table = hit_table(seed, sigs)
    print(table)
    with open(out + "/hits.txt", "w") as f:
        f.write(table + "\n")
    protos = [proto(i, p, r) + ";" for i, (p, r, _, _) in enumerate(sigs)]
    with open(out + "/callee.c", "w") as f:
        f.write("/* seed %d */\n" % seed + HEADER + "\n")
        for i, (p, r, raws, _) in enumerate(sigs):
            f.write(callee(i, p, r, raws) + "\n\n")
    with open(out + "/caller.c", "w") as f:
        f.write("/* seed %d */\n" % seed + HEADER + "\n".join(protos) + "\n\n")
        for i, (p, r, raws, modes) in enumerate(sigs):
            f.write(caller(i, p, r, raws, modes) + "\n\n")
    with open(out + "/driver.c", "w") as f:
        decls = "\n".join("int caller%d(void);" % i for i in range(count))
        calls = "\n".join("    caller%d();" % i for i in range(count))
        f.write(DRIVER.replace("%DECLS%", decls).replace("%CALLS%", calls).replace("%N%", str(count)))


if __name__ == "__main__":
    main()
