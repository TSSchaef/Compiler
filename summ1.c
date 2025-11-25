int main()
{
    int a, b;
    putstring("Enter two integers\n");
    a = getint();
    b = getint();
    putint(a);
    putstring(" + ");
    putint(b);
    putstring(" = ");

    putint(a+b);
    putchar(10);
    return 0;
}
