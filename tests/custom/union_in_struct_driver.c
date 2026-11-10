int tagged_size(void); int tagged_offset(void); int tagged_use(void); int tagged_float(void);
int either_size(void); int either_members(void); int either_overlap(void);
int wrap_size(void); int wrap_bytes(void);

int main()
{
    if (tagged_size() != 12) return 1;
    if (tagged_offset() != 4) return 2;
    if (tagged_use() != 51) return 3;
    if (tagged_float() != 6) return 4;
    if (either_size() != 8) return 5;
    if (either_members() != 34) return 6;
    if (either_overlap() != 1) return 7;
    if (wrap_size() != 12) return 8;
    if (wrap_bytes() != 1104) return 9;
    return 0;
}
