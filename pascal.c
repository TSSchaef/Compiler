
/*
 * Kind of a stress test.
 * All this ugliness would go away though if we had loops.
 */

int row[10];

void show1(int i, int space)
{
    putint(row[i]);
    putchar(space);
}

void show2(int i, int space)
{
    show1(i, 32);
    show1(i+1, space);
}

void show3(int i, int space)
{
    show1(i, 32);
    show2(i+1, space);
}

void show4(int i, int space)
{
    show2(i, 32);
    show2(i+2, space);
}

void show5(int i, int space)
{
    show2(i, 32);
    show3(i+2, space);
}

void show6(int i, int space)
{
    show3(i, 32);
    show3(i+3, space);
}

void show7(int i, int space)
{
    show3(i, 32);
    show4(i+3, space);
}

void show8(int i, int space)
{
    show4(i, 32);
    show4(i+4, space);
}

void show9(int i, int space)
{
    show4(i, 32);
    show5(i+4, space);
}

void show10(int i, int space)
{
    show5(i, 32);
    show5(i+5, space);
}

int update(int add, int i)
{
    int old;
    old = row[i];
    row[i] += add;
    return old;
}

void update_row()
{
    update(update(update(update(update(update(update(update(update(1, 1), 2), 3), 4), 5), 6), 7), 8), 9);
}

int main()
{
    row[0] = 1;
    row[1] = row[2] = row[3] = row[4] = row[5] = row[6] = row[7] = row[8] = 0;

    show1(0, 10);
    update_row(); show2(0, 10);
    update_row(); show3(0, 10);
    update_row(); show4(0, 10);
    update_row(); show5(0, 10);
    update_row(); show6(0, 10);
    update_row(); show7(0, 10);
    update_row(); show8(0, 10);
    update_row(); show9(0, 10);
    update_row(); show10(0, 10);
    return row[0];
}
