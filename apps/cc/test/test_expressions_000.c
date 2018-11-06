const int g_a = 123;
const int g_b = (g_a + g_a) * g_a;

int f(int a)
{
    int i = a + g_a;
    int j;

    j = i * g_a;

    return (j + i) / 3;
}
