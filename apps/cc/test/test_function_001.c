int fac(int n)
{
    if(n < 2)
        return 1;

    return n * fac(n - 1);
}

int main()
{
    return fac(4);
}
