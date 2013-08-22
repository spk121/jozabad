SRC = \
    lib.c \
    parch_broker.c \
    parch_msg.c \
    parch_msg2.c \
    parch_node.c \
    parch_state_engine.c \
    test_node1.c

ISRC := $(patsubst %.c,%.o,$(SRC))

OBJ := $(patsubst %.c,%.o,$(SRC))

CPPFLAGS = \
    -I../include -I/usr/local/include \
    -Wall \
    -Wunused-macros \
    -Wendif-labels \
    -pedantic \
    -std=c99

CFLAGS = \
    -fstrict-aliasing \
    -fstrict-overflow \
    -ftree-vrp \
    -ggdb3 \
    -march=native \
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
    -Wunreachable-code

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

all: parch_broker test_node
#	tmp_broker_med tmp_broker_small

lib.i : lib.c
parch_broker.i : parch_broker.c
parch_msg.i : parch_msg.c
parch_msg2.i : parch_msg2.c
parch_state_engine.i : parch_state_engine.c
parch_throughput.i : parch_throughput.c
parch_packet.i : parch_packet.c
parch_node.i : parch_node.c
parch_window.i : parch_window.c
test_node1.i : test_node1.c

lib.i: lib.c
	cpp $(CPPFLAGS) $< $@

lib.o: lib.i
	$(CC) -c $(CFLAGS) $< -o $@

parch_msg.c: parch_msg.xml codec_c.gsl
	./generate

test_node: parch_node.o test_node1.o parch_msg.o
	$(CC) -o $@ $^ $(LDFLAGS) -lczmq -lzmq

parch_broker: lib.o parch_broker.o parch_msg.o parch_msg2.o parch_state_engine.o parch_throughput.o \
	parch_packet.o parch_window.o
	$(CC) -o $@ $^ $(LDFLAGS_MEDIUM) -lczmq -lzmq

clean:
	-rm test_node parch_broker *.o *.i

.PHONY: check-syntax

check-syntax:
	$(CC) -Wall -Wextra -pedantic -fsyntax-only $(SRCS)

