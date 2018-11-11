struct A {
    int a;
    char b;
    //int c[10];
};

struct A g_a;

void f()
{
    struct A a;
    const struct A b;

    a.a = 1;
    //a.c[1] = 1;

    g_a.a = a.a;
}

void f1(struct A *i)
{
    i->a = 1;
    (*i).a = 2;
}

