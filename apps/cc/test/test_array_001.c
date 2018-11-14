int main()
{
    int a[10];
    int i;
    int b[10];
    int c = 0;

    for(i = 0; i < 10; ++i)
        a[i] = i;

    for(i = 0; i < 10; ++i)
        b[i] = a[i];

    for(i = 0; i < 10; ++i)
        c = c + b[i];

    return c;
}
