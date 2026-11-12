#!/usr/bin/env python3
"""Random ABI cross-check generator (integer, pointer, float, double, struct and union signatures).

Prints, per run, how many signatures hit each ilp32d case (see classify).

Writes three C files into OUT_DIR:
  callee.c  N functions f<i>(params) -> ret. Each checks every parameter
            against its expected value and calls abi_fail(i, k) on a mismatch,
            then returns a value derived from the parameters.
  caller.c  N functions caller<i>(void) that call f<i> with the expected values
            (as literals, as same-type locals, or as int locals converted at the
            call) and check the return value.
  driver.c  main, abi_fail, abi_ptr_c, abi_ptr_i, abi_int (always built by GCC).
Usage: gen_abi.py SEED COUNT OUT_DIR
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
RECORDS = {
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
SCALAR_LAYOUT = {  # name: (size, align, class) with class i(nteger), p(ointer), f(loating)
    "int": (4, 4, "i"), "unsigned": (4, 4, "i"), "char": (1, 1, "i"), "signed char": (1, 1, "i"),
    "unsigned char": (1, 1, "i"), "short": (2, 2, "i"), "unsigned short": (2, 2, "i"),
    "int *": (4, 4, "p"), "char *": (4, 4, "p"), "float": (4, 4, "f"), "double": (8, 8, "f"),
}


def round_up(v, m):
    return (v + m - 1) // m * m


def fields_of(t):
    """(name, type, count) for each field of record t."""
    return [(f[0], f[1], f[2] if len(f) > 2 else None) for f in RECORDS[t][1]]


def layout(t):
    """(size, align) of a scalar or record type, by the C layout rules."""
    if t in SCALAR_LAYOUT:
        return SCALAR_LAYOUT[t][:2]
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


STRUCTS = {t: (decl(t), leaves(t)) for t in RECORDS}
STRUCT_NAMES = list(STRUCTS)
SCALAR_TYPES = list(INT_TYPES) + PTR_TYPES + FLOAT_TYPES
PARAM_TYPES = SCALAR_TYPES + STRUCT_NAMES
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


def split_params(params, ret="void"):
    """Indexes of the parameters (doubles, structs and unions) that ilp32d splits
    between a7 and the stack (see classify)."""
    return classify(params, ret)[1]


def gen_sig(rng):
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


HEADER = ("void abi_fail(int sig, int param);\nchar *abi_ptr_c(void);\nint *abi_ptr_i(void);\nint abi_int(int x);\n"
          + "\n".join(decl for decl, _ in STRUCTS.values()) + "\n")

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
    for params, ret, _, _ in sigs:
        hits = classify(params, ret)[0]
        occurrences.update(hits)
        signatures.update(set(hits))
    lines = ["hits seed %d (signatures / occurrences):" % seed]
    lines += ["  %-8s %4d / %4d  %s" % (key, signatures[key], occurrences[key], text) for key, text in CASES]
    missing = [key for key, _ in CASES if not occurrences[key]]
    lines.append("  NOT HIT: " + " ".join(missing) if missing else "  every case hit")
    return "\n".join(lines)


def main():
    seed, count, out = int(sys.argv[1]), int(sys.argv[2]), sys.argv[3]
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
