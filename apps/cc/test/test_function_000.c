void f();

// empty
void f1() { }

// types
int f2() { }

char f3();

short f4();

// parameters
void f5(int a);

void f6(int a, void *b);

// only declarations
void f7(int a)
{
    int i;
    int j = 0;
}

// only statements
int f8(int a)
{
    a = a * 3;

    return a;
}

// declarations + statements
void f9(int a)
{
    int i;

    i = 0;

    i += 3 * a;
}