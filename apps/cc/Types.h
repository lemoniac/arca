#ifndef CC__TYPES__H
#define CC__TYPES__H

#include <string>

enum class Type { Error = -1, Unknown, Void, Int, Char, Short, Function, Struct, Union };

struct DeclarationSpecifier {

    Type type;
    std::string structName;
    bool isConst = false;
    bool isSigned = true;
    bool isPointer = false;
    bool isStatic = false;
    bool isVolatile = false;
};

int size(Type type);

#endif//CC__TYPES__H
