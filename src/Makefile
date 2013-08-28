INCLUDES = \
    ../include/action.h \
    ../include/channel.h \
    ../include/connections.h \
    ../include/diagnostic.h \
    ../include/direction.h \
    ../include/flow.h \
    ../include/lib.h \
    ../include/log.h \
    ../include/packet.h \
    ../include/poll.h \
    ../include/state.h \
    ../include/parch_msg.h \
    ../include/throughput.h \
    ../include/window.h

SRC = action.o \
    channel.o \
    connections.o \
    flow.o \
    diagnostic.o \
    direction.o \
    lib.o \
    main.o \
    packet.o \
    poll.o \
    state.o \
    parch_msg.o \
    throughput.o \
    window.o \

#ISRC := $(patsubst %.c,%.o,$(SRC))

OBJ := $(patsubst %.c,%.o,$(SRC))

CPPFLAGS = \
    -I/usr/local/include \
    -Wall \
    -Wunused-macros \
    -Wendif-labels \
    -pedantic \
    -std=c99

CFLAGS = \
    $(CPPFLAGS) \
    -fstrict-aliasing \
    -fstrict-overflow \
    -ftree-vrp \
    -ggdb3 \
    -march=native \
    -O2 \
    -std=c99 \
    -Wall \
    -Warray-bounds \
    -Wc++-compat \
    -Wcast-align \
    -Wno-cast-qual \
    -Wextra \
    -Wmissing-prototypes \
    -Wmissing-declarations \
    -Wpointer-arith \
    -Wstrict-aliasing \
    -Wstrict-overflow=5 \
    -Wundef \
    -Wunreachable-code \
    -Winvalid-pch

#    -fmudflapth \


CC = gcc
# CC = /home/mike/studio/SolarisStudio12.3-linux-x86-bin/solarisstudio12.3/bin/cc
# CC = /usr/lib/clang-analyzer/scan-build/ccc-analyzer
# CFLAGS = -std=c99  -Wall -Wextra -g -O0

# compile the data but tell the compiler to separate the code into
# separate sections within the translation unit. This will be done for
# functions, classes, and external variables by using the following
# two compiler flags:

CFLAGS += -fdata-sections -ffunction-sections

LDFLAGS_LARGE = --verbose -Wl,-L/usr/local/lib
LDFLAGS_MEDIUM = -Wl,--gc-sections --verbose
LDFLAGS_SMALL = -Wl,--gc-sections -Wl,--strip-all --verbose

all: broker test_node
#	tmp_broker_med tmp_broker_small

# lib.i : lib.c
# parch_broker.i : parch_broker.c
# parch_msg.i : parch_msg.c
# parch_msg2.i : parch_msg2.c
# parch_state_engine.i : parch_state_engine.c
# parch_throughput.i : parch_throughput.c
# parch_packet.i : parch_packet.c
# parch_node.i : parch_node.c
# parch_window.i : parch_window.c
# test_node1.i : test_node1.c
# poll.i : poll.c

################################################3
# source files

parch_msg.c: parch_msg.xml codec_c.gsl
../include/parch_msg.h: parch_msg.xml codec_c.gsl
	./generate

################################################3
# header files

../include/svc.h.gch : ../include/svc.h $(INCLUDES)
	$(CC) $(CFLAGS) $<

################################################3
# object files

action.o : action.c ../include/svc.h.gch
	$(CC) $(CFLAGS) -c -o $@ $<

channel.o : channel.c
	$(CC) $(CFLAGS) -Wno-missing-field-initializers -c -o $@ $<

connections.o : connections.c
	$(CC) $(CFLAGS) -c -o $@ $<

diagnostic.o : diagnostic.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o : main.c
	$(CC) $(CFLAGS) -c -o $@ $<

lib.o : lib.c
	$(CC) $(CFLAGS) -c -o $@ $<

direction.o : direction.c
	$(CC) $(CFLAGS) -c -o $@ $<

flow.o : flow.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Auto-generated file would have too many warnings if warnings were enabled.
parch_msg.o : parch_msg.c
	$(CC) -c -o $@ $<

packet.o : packet.c
	$(CC) $(CFLAGS) -c -o $@ $<

poll.o : poll.c
	$(CC) $(CFLAGS) -c -o $@ $<

state.o : state.c
	$(CC) $(CFLAGS) -c -o $@ $<

throughput.o : throughput.c
	$(CC) $(CFLAGS) -c -o $@ $<

window.o : window.c
	$(CC) $(CFLAGS) -c -o $@ $<

################################################3
# executable files


test_node: parch_node.o test_node1.o parch_msg.o
	$(CC) -o $@ $^ $(LDFLAGS) -lczmq -lzmq

broker: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS_MEDIUM) -lczmq -lzmq

clean:
	-rm ../include/svc.h.gch
	-rm $(OBJ) test_node1.o parch_node.o
	-rm test_node broker

.PHONY: check-syntax

check-syntax:
	$(CC) -Wall -Wextra -pedantic -fsyntax-only $(SRCS) $(ISRCS)

