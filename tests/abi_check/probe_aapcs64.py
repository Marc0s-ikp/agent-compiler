#!/usr/bin/env python3
"""Checks the AAPCS64 model of gen_abi.py against what GCC's caller really does.

GCC to GCC passing cannot show that the model is right (two compilers can agree and both be wrong), so this
tool asks GCC's -O2 caller where it puts each argument: a hand-written assembly callee `probe` stores x0-x8,
d0-d7 and 96 words from sp into globals, and returns known patterns in x0, x1, d0-d3 (and through [x8] when the
model says the result is in memory). The generated caller calls it through a cast to the signature under test,
with arguments whose bytes are known, and prints the globals, the bytes behind any by-reference pointer and
the bytes of the result. This script compares all of that with the placement the model predicts.

Usage (needs aarch64-linux-gnu-gcc and qemu-aarch64, so inside the Docker image):
    probe_aapcs64.py SEED COUNT OUT_DIR
It prints, for each case of AAPCS64_CASES, the first signature that hit it and whether GCC agreed, and exits 1 on
any disagreement.
"""
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_abi as g  # noqa: E402

STACK_WORDS = 96
WORDS = [bytes(0x10 * (k + 1) + j for j in range(8)) for k in range(8)]  # what the probe returns in x0, x1, d0-d3, [x8]

STUB = r"""
    .text
    .globl probe
probe:
    adrp x9, probe_x
    add  x9, x9, :lo12:probe_x
    stp  x0, x1, [x9, #0]
    stp  x2, x3, [x9, #16]
    stp  x4, x5, [x9, #32]
    stp  x6, x7, [x9, #48]
    str  x8, [x9, #64]
    adrp x10, probe_v
    add  x10, x10, :lo12:probe_v
    stp  d0, d1, [x10, #0]
    stp  d2, d3, [x10, #16]
    stp  d4, d5, [x10, #32]
    stp  d6, d7, [x10, #48]
    adrp x11, probe_s
    add  x11, x11, :lo12:probe_s
    mov  x12, sp
    mov  x13, #0
1:  ldr  x14, [x12, x13, lsl #3]
    str  x14, [x11, x13, lsl #3]
    add  x13, x13, #1
    cmp  x13, #%STACK%
    b.ne 1b
    adrp x9, probe_mem
    ldr  w9, [x9, :lo12:probe_mem]
    cbz  w9, 2f
%MEM%
2:
%REGS%
    ret
"""


def stub():
    mem = "\n".join("    ldr x14, =0x%s\n    str x14, [x8, #%d]" % (w[::-1].hex(), 8 * k) for k, w in enumerate(WORDS))
    regs = "    ldr x0, =0x%s\n    ldr x1, =0x%s\n" % (WORDS[0][::-1].hex(), WORDS[1][::-1].hex())
    regs += "".join("    ldr x14, =0x%s\n    fmov d%d, x14\n" % (WORDS[2 + k][::-1].hex(), k) for k in range(4))
    return STUB.replace("%STACK%", str(STACK_WORDS)).replace("%MEM%", mem).replace("%REGS%", regs)


def pattern(k, size):
    return bytes(((k + 1) * 29 + j * 7 + 3) & 0xFF for j in range(size))


def c_bytes(b):
    return "{" + ", ".join(str(x) for x in b) + "}"


def where_of(i, k, place):
    """The C expression of the word that holds the pointer of a by-reference argument."""
    return "probe_x[%d]" % place.regs[0] if place.where == "x" else "probe_s[%d]" % (place.offset // 8)


def sig_code(i, params, ret):
    """The C function that calls the probe with signature i and prints what it recorded."""
    st = g.Aapcs64State()
    places = [g.aapcs64_place(st, t) for t in params]
    mem = ret != "void" and g.aapcs64_result(ret)[0] == "mem"
    out = ["static void run%d(void)\n{" % i]
    out += ["    static const unsigned char pat%d[] = %s;\n    %s a%d;" % (k, c_bytes(pattern(k, g.layout(t)[0])), t, k)
            for k, t in enumerate(params)]
    out += ["    unsigned char ref[64], res[64];"]
    out += ["    memcpy(&a%d, pat%d, sizeof a%d);" % (k, k, k) for k in range(len(params))]
    ptype = "%s (*)(%s)" % (ret, ", ".join(params) or "void")
    args = ", ".join("a%d" % k for k in range(len(params)))
    out += ["    %s r;" % ret] if ret != "void" else []
    out.append("    probe_mem = %d;" % mem)
    out.append("    %s((%s)probe)(%s);" % ("r = " if ret != "void" else "", ptype, args))
    out.append('    printf("sig %d\\n");' % i)
    out.append('    dump("x", probe_x, 9); dump("v", probe_v, 8); dump("s", probe_s, %d);' % STACK_WORDS)
    for k, (t, p) in enumerate(zip(params, places)):
        if p.by_ref:
            out.append('    memcpy(ref, (void *)%s, %d); printf("ref %d "); hex(ref, %d);' % (where_of(i, k, p), g.layout(t)[0], k, g.layout(t)[0]))
    if ret != "void":
        out.append('    memcpy(res, &r, sizeof r); printf("ret "); hex(res, sizeof r);')
    out.append("}")
    return "\n".join(out)


HEAD = r"""#include <stdio.h>
#include <string.h>
extern void probe(void);
unsigned long probe_x[9], probe_v[8], probe_s[%STACK%];
int probe_mem;
static void hex(const unsigned char *p, int n) { int i; for (i = 0; i < n; i++) printf("%02x", p[i]); printf("\n"); }
static void dump(const char *name, const unsigned long *w, int n)
{
    int i;
    printf("%s", name);
    for (i = 0; i < n; i++) printf(" %016lx", w[i]);
    printf("\n");
}
"""


def parse(text):
    """{sig: {'x': [bytes], 'v': [bytes], 's': [bytes], 'ref': {k: bytes}, 'ret': bytes}}"""
    sigs, cur = {}, None
    for line in text.splitlines():
        f = line.split()
        if f[0] == "sig":
            cur = sigs[int(f[1])] = {"ref": {}}
        elif f[0] in "xvs" and len(f[0]) == 1:
            cur[f[0]] = [bytes.fromhex(w)[::-1] for w in f[1:]]
        elif f[0] == "ref":
            cur["ref"][int(f[1])] = bytes.fromhex(f[2])
        elif f[0] == "ret":
            cur["ret"] = bytes.fromhex(f[1])
    return sigs


def check_arg(k, t, place, rec, errors):
    size = g.layout(t)[0]
    image = pattern(k, size)
    if place.by_ref:
        word = rec["x"][place.regs[0]] if place.where == "x" else rec["s"][place.offset // 8]
        if rec["ref"][k] != image or int.from_bytes(word, "little") == 0:
            errors.append("param %d: copy behind the pointer differs" % k)
        return
    if place.where == "x":
        got = b"".join(rec["x"][r] for r in place.regs)[:size]
        want = image[:size]
    elif place.where == "v":
        base = g.hfa_of(t)[0] if t in g.RECORDS else t
        bs = g.layout(base)[0]
        got = b"".join(rec["v"][r][:bs] for r in place.regs)
        want = image[:bs * len(place.regs)]
    else:
        got = b"".join(rec["s"])[place.offset:place.offset + size]
        want = image
    if got != want:
        errors.append("param %d (%s): model says %s %s, GCC's bytes differ" % (k, t, place.where, place.regs or place.offset))


def check_ret(ret, rec, errors):
    if ret == "void":
        return
    size = g.layout(ret)[0]
    where, regs = g.aapcs64_result(ret)
    if where == "mem":
        want = b"".join(WORDS)[:size]
        if int.from_bytes(rec["x"][8], "little") == 0:
            errors.append("result: x8 is zero")
    elif where == "x":
        want = (WORDS[0] + WORDS[1])[:size]
    else:
        base = g.hfa_of(ret)[0] if ret in g.RECORDS else ret
        bs = g.layout(base)[0]
        want = b"".join(WORDS[2 + r][:bs] for r in regs)
    if rec["ret"][:size] != want:
        errors.append("result (%s): model says %s %s, GCC's bytes differ" % (ret, where, regs))


def main():
    import random
    seed, count, out = int(sys.argv[1]), int(sys.argv[2]), sys.argv[3]
    g.setup_target("aarch64")
    rng = random.Random(seed)
    sigs = [g.gen_sig_aarch64(rng)[:2] for _ in range(count)]
    os.makedirs(out, exist_ok=True)
    src = HEAD.replace("%STACK%", str(STACK_WORDS)) + "\n".join(g.HEADER.splitlines()[4:]) + "\n"
    src += "\n".join(sig_code(i, p, r) for i, (p, r) in enumerate(sigs))
    src += "\nint main(void)\n{\n" + "\n".join("    run%d();" % i for i in range(count)) + "\n    return 0;\n}\n"
    with open(out + "/probe.c", "w") as f:
        f.write(src)
    with open(out + "/stub.S", "w") as f:
        f.write(stub())
    subprocess.run(["aarch64-linux-gnu-gcc", "-O2", "-w", "-static", "-o", out + "/probe.elf", out + "/probe.c", out + "/stub.S"], check=True)
    run = subprocess.run(["qemu-aarch64", out + "/probe.elf"], capture_output=True, text=True, check=True)
    recs = parse(run.stdout)
    first, bad = {}, 0
    for i, (params, ret) in enumerate(sigs):
        errors = []
        st = g.Aapcs64State()
        for k, t in enumerate(params):
            check_arg(k, t, g.aapcs64_place(st, t), recs[i], errors)
        check_ret(ret, recs[i], errors)
        bad += bool(errors)
        for e in errors:
            print("sig %d (%s) %s: %s" % (i, ", ".join(params), ret, e))
        for case in g.classify_aapcs64(params, ret)[0]:
            first.setdefault(case, (i, not errors))
    for case, text in g.AAPCS64_CASES:
        i, ok = first.get(case, (None, False))
        sig = "NOT HIT" if i is None else "sig %-3d %-4s %s(%s)" % (i, "ok" if ok else "BAD", sigs[i][1], ", ".join(sigs[i][0]))
        print("%-12s %s" % (case, sig))
    print("probe: %d signatures, %d disagree with the model" % (count, bad))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
