#ifndef CC__STRUCT__H
#define CC__STRUCT__H

#include <memory>
#include <string>
#include <vector>
#include "Types.h"

class Struct {
public:
    struct Member {
        DeclarationSpecifier declSpec;
        std::string name;
    };

    std::string name;
    std::string varname;
    std::vector<Member> member;

    int size() const { return member.size() * 4; }
};

typedef std::shared_ptr<Struct> StructPtr;

#endif//CC__STRUCT__H
