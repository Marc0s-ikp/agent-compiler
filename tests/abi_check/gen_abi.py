#!/usr/bin/env python3
"""Random ABI cross-check generator (integer, pointer, float and double signatures).

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
PARAM_TYPES = list(INT_TYPES) + PTR_TYPES + FLOAT_TYPES
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
    want = struct.pack("<d", v)
    tests = " || ".join("b[%d] != %d" % (n, c) for n, c in enumerate(want))
    return "    b = (unsigned char *)&p%d;\n    if (%s) abi_fail(%%d, %d);" % (k, tests, k)


def ptr_getter(t):
    return "abi_ptr_i()" if t == "int *" else "abi_ptr_c()"


def gen_params(rng):
    """Half the signatures are uniform draws (0 to 14 parameters). A uniform
    draw almost never uses up fa0-fa7, so the other half are shaped:
    'fp_heavy' has 9 to 12 float/double parameters among 0 to 8 others (floats
    and doubles in a registers, doubles in pairs and on the stack); 'split' has
    8 float/double and 7 other parameters in any order, then a double (low
    word in a7, high word on the stack), then 0 to 3 more parameters."""
    shape = rng.choice(["random", "random", "fp_heavy", "split"])
    others = list(INT_TYPES) + PTR_TYPES
    if shape == "random":
        return [rng.choice(PARAM_TYPES) for _ in range(rng.randint(0, 14))]
    if shape == "fp_heavy":
        params = [rng.choice(FLOAT_TYPES) for _ in range(rng.randint(9, 12))]
        params += [rng.choice(others) for _ in range(rng.randint(0, 8))]
        rng.shuffle(params)
        return params
    params = [rng.choice(FLOAT_TYPES) for _ in range(8)] + [rng.choice(others) for _ in range(7)]
    rng.shuffle(params)
    return params + ["double"] + [rng.choice(PARAM_TYPES) for _ in range(rng.randint(0, 3))]


def split_params(params):
    """Indexes of the doubles that ilp32d splits between a7 and the stack:
    fa0-fa7 are used up and only a7 is left. (A float or double takes an fa
    register while one is free, then follows the integer rules; a double needs
    two a registers, or one if only a7 is left, which is the split.)"""
    fa = ia = 0
    split = set()
    for k, t in enumerate(params):
        if t in FLOAT_TYPES and fa < 8:
            fa += 1
        elif t == "double":
            if ia == 7:
                split.add(k)
            ia = min(ia + 2, 8)
        else:
            ia = min(ia + 1, 8)
    return split


def gen_sig(rng):
    params = gen_params(rng)
    ret = rng.choice(PARAM_TYPES + ["void"])
    raws = [rng.choice(FEDGE if t in FLOAT_TYPES else EDGE) for t in params]
    modes = [rng.choice(["lit", "same", "int", "call"]) for _ in params]
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
    split = split_params(params)
    same = [k for k, t in enumerate(params) if t == ret and k not in split]
    return same[-1] if same else None


def callee(i, params, ret, raws):
    out = [proto(i, params, ret), "{"]
    split = split_params(params)
    if split:
        out.append("    unsigned char *b;")
    if ret in INT_TYPES:
        out.append("    unsigned acc;")
        out.append("    acc = 0;")
    for k, (t, r) in enumerate(zip(params, raws)):
        if k in split:
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
        if t in FLOAT_TYPES:
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
        elif ret in FLOAT_TYPES:
            k = returned_param(params, ret)
            want = flit(raws[k], ret) if k is not None else flit(2.5, ret)
            out.append("    if (r != %s) abi_fail(%d, 99);" % (want, i))
        else:
            out.append("    if (r != %s) abi_fail(%d, 99);" % (ptr_getter(ret), i))
    out.append("    return 0;")
    out.append("}")
    return "\n".join(out)


HEADER = "void abi_fail(int sig, int param);\nchar *abi_ptr_c(void);\nint *abi_ptr_i(void);\nint abi_int(int x);\n"

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


def main():
    seed, count, out = int(sys.argv[1]), int(sys.argv[2]), sys.argv[3]
    rng = random.Random(seed)
    sigs = [gen_sig(rng) for _ in range(count)]
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
