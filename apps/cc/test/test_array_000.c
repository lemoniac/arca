int g_a[10];
char g_b[8];
short g_c[3];

void f()
{
    int a[10];
    int i = a[1];
    int offset = 2;
    int j = a[offset];

    i = a[i * 2];
    a[j] = a[i];
}
