#ifndef ARCA__ASM__STRUCT__H
#define ARCA__ASM__STRUCT__H

#include <string>
#include <vector>
#include "types.h"

struct Struct {
    struct Member {
        Member(std::string &&name, Type type, unsigned elem):
            name(std::move(name)), type(type), elem(elem)
        {}

        std::string name;
        Type type;
        unsigned elem;
        unsigned offset = 0;

        unsigned size() const;
    };

    std::vector<Member> member;
    int size = -1;

    int getSize();
    int getOffset(const std::string &name);
};

#endif//ARCA__ASM__STRUCT__H
