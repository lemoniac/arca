int *TEXT_RAM = 0x20000000;

int main()
{
    const int green = 2 << 8;
    (*TEXT_RAM) = 'A' | green;

    return 0;
}
