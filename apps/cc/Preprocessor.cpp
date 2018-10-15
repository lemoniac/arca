#include <stdio.h>
#include <string.h>
#include <utility>
#include "Preprocessor.h"

void Preprocessor::preprocess(const char *filename)
{
    file = fopen(filename, "rt");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = new char[size + 1];
    size2 = size;
    buffer2 = new char[size + 1];
    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    phase2and3();
    phase4();
}

Preprocessor::~Preprocessor()
{
    if(file)
        fclose(file);
    if(buffer)
        delete [] buffer;
}

void Preprocessor::phase2and3()
{
    // combine lines
    size_t j = 0;
    for(size_t i = 0; i < size - 1; i++)
    {
        if(buffer[i] == '\\' && buffer[i+1] == '\n')
        {
            size2 -= 2;
            i++;
        }
        else
            buffer2[j++] = buffer[i];
    }

    buffer2[j] = buffer[size - 1];
    buffer2[j+1] = 0;

    printf("***%s###", buffer2);
    printf("%lu %lu\n", size2, strlen(buffer2));

    std::swap(buffer, buffer2);
    std::swap(size, size2);

    // remove stuff
    j = 0;
    for(size_t i = 0; i < size - 1; i++)
    {
        if(buffer[i] == '/' && buffer[i+1] == '/') // remove // comments
        {
            i+= 2;
            size2-= 2;
            while(i < size && buffer[i] != '\n')
            {
                i++;
                size2--;
            }
        }
        else if(buffer[i] == '/' && buffer[i+1] == '*') // remove /* */ comments
        {
            i+= 2;
            size2-= 2;
            while(i < (size - 1))
            {
                if(buffer[i] == '*' && buffer[i+1] == '/')
                    break;
                i++;
                size2--;
            }
            i++;
        }
        else if(buffer[i] == '\n' && isSpace(buffer[i+1])) // remove spaces at the begining of lines
        {
            buffer2[j++] = '\n';
            i+= 2;
            while(i < size - 1 && isSpace(buffer[i])) i++;
            i--;
        }
        else if(isSpace(buffer[i]) && isSpace(buffer[i+1])) // collapse multiple whitespaces into one
        {
            buffer2[j++] = ' ';
            while(i < size - 1 && isSpace(buffer[i])) i++;
            i--;
        }
        else
            buffer2[j++] = buffer[i];
    }
    buffer2[j] = buffer[size - 1];
    buffer2[j+1] = 0;

    printf("***%s###", buffer2);
    size2 = strlen(buffer2);

    std::swap(buffer, buffer2);
    std::swap(size, size2);
}

void Preprocessor::phase4()
{
}

bool Preprocessor::isSpace(const char c) const
{
    return c == ' ' || c == '\t' || c == '\v';
}

