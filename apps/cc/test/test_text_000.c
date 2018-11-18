int *TEXT_RAM = 0x20000000;

int main()
{
    (*TEXT_RAM) = 'A';

    return 0;
}
