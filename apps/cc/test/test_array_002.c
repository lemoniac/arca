int g_a[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

int main()
{
    int i;
    int c = 0;

    for(i = 0; i < 10; ++i)
        c = c + g_a[i];

    return c;
}
