CPPFLAGS=-std=c++14 -g
OTHER=lex.yy.c
DEPS=Expression.h Statement.h
OBJS=Function.o CodeGenerator.o Statement.o \
	PrintVisitor.o SymbolTablePass.o Expression.o Parser.o \
	Preprocessor.o Variable.o

cc: cc.cpp lex.yy.c $(OBJS) $(DEPS)
	$(CXX) -g -o cc cc.cpp $(OBJS) $(CPPFLAGS)

lex.yy.c: c.flex
	flex c.flex

clean:
	rm -f cc
	rm -f $(OTHER) $(OBJS)
