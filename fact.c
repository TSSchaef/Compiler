
int F;

void print(int i, int f)
{
    putint(i);
    putchar((int)'!');
    putchar((int)' ');
    putchar((int)'=');
    putchar((int)' ');
    putint(f);
    putchar(10);
}

int main()
{
    F=1;
    print(1, F);
    print(2, F*=2);
    print(3, F*=3);
    print(4, F*=4);
    print(5, F*=5);
    print(6, F*=6);
    print(7, F*=7);
    print(8, F*=8);
    print(9, F*=9);
    return F;
}
