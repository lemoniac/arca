CPPFLAGS=--std=c++11 -I../../filesystem
OBJS=../../filesystem/filesystem.o

all: cp ls rm

cp: cp.cpp $(OBJS)
	g++ -g -o cp cp.cpp $(OBJS) $(CPPFLAGS)

ls: ls.cpp $(OBJS)
	g++ -g -o ls ls.cpp $(OBJS) $(CPPFLAGS)

rm: rm.cpp $(OBJS)
	g++ -g -o rm rm.cpp $(OBJS) $(CPPFLAGS)

clean:
	rm -f cp ls
