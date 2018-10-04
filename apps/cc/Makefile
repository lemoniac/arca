CPPFLAGS=-std=c++14
OTHER=lex.yy.c
DEPS=Expression.h Statement.h
OBJS=Function.o

cc: cc.cpp lex.yy.c $(OBJS) $(DEPS)
	g++ -g -o cc cc.cpp $(OBJS) $(CPPFLAGS)

lex.yy.c: c.flex
	#bison c.y -d
	flex c.flex

clean:
	rm -f cc
	rm -f $(OTHER) $(OBJS)
