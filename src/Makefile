INCLUDES = \
    action.h \
    connections.h \
    dname.h \
    flow.h \
    lib.h \
    poll.h \
    state.h \
    msg.h

VPATH = ../libjoza

CSRC = action.c \
	channel.c \
	diag.c \
	lcn.c \
	lib.c \
        log.c \
	main.c \
	packet.c \
	poll.c \
	raii.c \
	state.c \
	tput.c \
	worker.c \
	joza_lib.c \
	joza_msg.c \
	msg.c \
	seq.c \
	iodir.c

CPPSRC = 

OBJ := $(patsubst %.c,%.o,$(CSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

CPPFLAGS = \
    -I/usr/local/include \
    -Wall \
    -Wunused-macros \
    -Wendif-labels \
    -pedantic \
    `pkg-config glib-2.0 --cflags-only-I`

CWARN = \
    -Wall \
    -Warray-bounds \
    -Wcast-align \
    -Wno-cast-qual \
    -Wextra \
    -Wmissing-declarations \
    -Wpointer-arith \
    -Wstrict-aliasing \
    -Wstrict-overflow=5 \
    -Wundef \
    -Wunreachable-code \
    -Winvalid-pch

CFLAGS = $(CPPFLAGS)\
    -fstrict-aliasing \
    -fstrict-overflow \
    -ftree-vrp \
    -ggdb \
    -march=native \
    -O0 \
    -std=gnu11 \
    `pkg-config glib-2.0 --cflogs-only-other` \
    $(CWARN)

CC = gcc
# CC = /home/mike/studio/SolarisStudio12.3-linux-x86-bin/solarisstudio12.3/bin/cc
# CC = /usr/lib/clang-analyzer/scan-build/ccc-analyzer
# CC = clang

# compile the data but tell the compiler to separate the code into
# separate sections within the translation unit. This will be done for
# functions, classes, and external variables by using the following
# two compiler flags:

ifeq (CC, gc)
CFLAGS += -fdata-sections -ffunction-sections
endif

LDFLAGS_LARGE = --verbose -Wl,-L/usr/local/lib
LDFLAGS_MEDIUM = -Wl,--gc-sections --verbose
LDFLAGS_SMALL = -Wl,--gc-sections -Wl,--strip-all --verbose

all: broker

################################################3
# header files


################################################3
# source files


################################################3
# object files
################################################3

# executable files


broker: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lczmq -lzmq -lm `pkg-config glib-2.0 --libs`

clean:
	-rm $(OBJ)
	-rm broker

.PHONY: check-syntax

check-syntax:
	$(CXX) -Wall -Wextra -pedantic -fsyntax-only $(CPPSRC)
	$(CC) -Wall -Wextra -pedantic -fsyntax-only $(CSRC)

