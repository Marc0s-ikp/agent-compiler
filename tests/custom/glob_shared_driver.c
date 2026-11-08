extern int counter;
int bump();
int peek();

int main()
{
    if (counter != 3) return 1;
    counter = 10;
    if (peek() != 10) return 2;
    if (bump() != 11) return 3;
    if (counter != 11) return 4;
    return 0;
}
