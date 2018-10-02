#include "struct.h"

unsigned Struct::Member::size() const
{
    switch(type)
    {
        case Type::Char: return elem;
        case Type::Int: return 4 * elem;
        case Type::UInt16: return 2 * elem;
    }

    return 0;
}

int Struct::getSize()
{
    if(size == -1)
    {
        size = 0;
        for(const auto &m : member)
            size += m.size();
    }

    return size;
}

int Struct::getOffset(const std::string &name)
{
    int off = 0;
    unsigned c = 0;

    for(const auto &m : member)
    {
        if(m.name == name)
            break;
        off += m.size();
        c++;
    }

    if(c == member.size())
        return -1;

    return off;
}
