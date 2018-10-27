cmake_minimum_required (VERSION 3.1)
project (cc)
set(CMAKE_CXX_STANDARD 14)
add_custom_target(
    lex
    COMMAND flex c.flex
    DEPENDS c.flex)
add_compile_options("-g")
add_executable(cc cc.cpp
    Function.cpp CodeGenerator.cpp Statement.cpp
    PrintVisitor.cpp SymbolTablePass.cpp Expression.cpp Parser.cpp
    Preprocessor.cpp Variable.cpp SimplifyExpressions.cpp SymbolTable.cpp)
add_dependencies(cc lex)