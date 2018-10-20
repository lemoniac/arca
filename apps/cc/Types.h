#ifndef CC__TYPES__H
#define CC__TYPES__H

enum class Type { Error = -1, Void, Int, Char, Function };

struct DeclarationSpecifier {

    Type type;
    bool isConst = false;
    bool isSigned = true;
    bool isPointer = false;
    bool isStatic = false;
    bool isVolatile = false;
};


#endif//CC__TYPES__H
