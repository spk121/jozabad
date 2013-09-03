swig -Wall -module jza -python joza_msg.i
gcc -g -D_LARGEFILE64_SOURCE -c -fpic joza_msg.c joza_msg_wrap.c -I/usr/include/python2.7
gcc -g -shared joza_msg.o joza_lib.o joza_msg_wrap.o -o _jza.so -lzmq -lczmq


# swig -Wall -module jza -perl5 joza_msg.i
# gcc -g -D_LARGEFILE64_SOURCE -c -fpic joza_msg.c joza_msg_wrap.c -I/usr/include/python2.7 -I/usr/lib/perl5/CORE
# gcc -g -shared joza_msg.o joza_lib.o joza_msg_wrap.o -o jza.so -lzmq -lczmq
