all: a.out

c.obj: ../libjoza/joza_msg.c ../libjoza/joza_lib.c
	clang -Wall -std=c11 -g -O0 \
	-o c.obj \
	../libjoza/joza_msg.c \
	../libjoza/joza_lib.c -lczmq -lzmq

cpp.obj: Poll.cxx process.cxx
	clang -Wall -std=c++11 -g -O0 \
	-o cpp.obj \
	Poll.cxx \
	process.cxx -lczmq -lzmq

joza_lib.o : ../libjoza/joza_lib.c
	clang -Wall -std=c11 -g -O0 -c $<

joza_msg.o : ../libjoza/joza_msg.c
	clang -Wall -std=c11 -g -O0 -c $<

%.o : %.cxx
	clang -Wall -std=c++11 -g -O0 -c $<

a.out: action.o joza_msg.o joza_lib.o Poll.o process.o Peer.o Channel.o
	clang -Wall -std=c++11 -g -O0 $^ -lstdc++ -lczmq -lzmq

clean:
	-rm c.obj cpp.obj
