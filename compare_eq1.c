
void comp(int x)
{
    putchar(61);
    if (x == 42) {
        putchar(89);
    } else {
        putchar(78);
    }
    putchar(10);
    return;
}

int main()
{
    comp(40);
    comp(41);
    comp(42);
    comp(43);
    comp(44);
    return 0;
}
