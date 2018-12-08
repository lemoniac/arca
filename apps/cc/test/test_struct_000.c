struct A {
    int a;
    char b;
    int c[10];
    int *d;
    const short e;
};

struct A g_a;

void f()
{
    struct A a;
    const struct A b;

    struct A c[10];

    a.a = 1;
    a.c[1] = 1;

    g_a.a = a.a;

    c[1].a = 1;
}

void f1(struct A *i)
{
    i->a = 1;
    (*i).a = 2;
}
