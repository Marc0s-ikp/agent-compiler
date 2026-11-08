enum E { A, B, C = 10, D, F = -1, G, H = 2 * 3, I = H + 1 };

int val(int which)
{
    if (which == 0) return A;
    if (which == 1) return B;
    if (which == 2) return C;
    if (which == 3) return D;
    if (which == 4) return F;
    if (which == 5) return G;
    if (which == 6) return H;
    return I;
}

enum E pick(enum E e)
{
    enum E local = C;
    if (e == A) return local;
    return e;
}

int variable_use()
{
    enum E v;
    v = D;
    v = v + 1;
    return v;
}
