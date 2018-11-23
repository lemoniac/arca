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

    int size() const
    {
        int s = 0;
        for(const auto &m : member)
            s += ::size(m.declSpec.type) * ((m.declSpec.elems > 0)? m.declSpec.elems : 1);
        return s;
    }
};

typedef std::shared_ptr<Struct> StructPtr;

#endif//CC__STRUCT__H
