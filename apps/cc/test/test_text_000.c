int main()
{
    int i;
    int *TEXT_RAM = 0x20000000;
    const int green = 2 << 8;

    for(i = 0; i < 26; ++i)
    {
        int *ptr = TEXT_RAM + (2 * i) + (80 * i);
        (*ptr) = ('A' + i) | green;
    }

    return 0;
}
