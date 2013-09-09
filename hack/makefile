all: a.out

c.obj: ../libjoza/joza_msg.c ../libjoza/joza_lib.c
	gcc -Wall -std=c11 -g -O0 \
	-o c.obj \
	../libjoza/joza_msg.c \
	../libjoza/joza_lib.c -lczmq -lzmq

cpp.obj: Poll.cxx process.cxx
	gcc -Wall -std=c++11 -g -O0 \
	-o cpp.obj \
	Poll.cxx \
	process.cxx -lczmq -lzmq

%.o : %.c
	gcc -Wall -std=c11 -g -O0 -c $<

%.o : %.cxx
	gcc -Wall -std=c++11 -g -O0 -c $<

a.out: joza_msg.o joza_lib.o Poll.o process.o Peer.o Channel.o
	gcc -Wall -std=c++11 -g -O0 $^ -lstdc++ -lczmq -lzmq

clean:
	-rm c.obj cpp.obj
