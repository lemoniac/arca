#ifndef CC__PREPROCESSOR__H
#define CC__PREPROCESSOR__H

struct _IO_FILE;
typedef struct _IO_FILE FILE;

class Preprocessor {
public:
    void preprocess(const char *filename);
    ~Preprocessor();

protected:
    void phase2and3();
    void phase4();

    bool isSpace(const char c) const;

    FILE *file = nullptr;
    char *buffer = nullptr;
    size_t size = 0;

    char *buffer2 = nullptr;
    size_t size2 = 0;
};

#endif//CC__PREPROCESSOR__H
