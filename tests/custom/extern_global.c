extern int from_driver;
extern int table[3];

int twice()
{
    return from_driver * 2;
}

int sum_table()
{
    from_driver = from_driver + 1;
    return table[0] + table[1] + table[2];
}
