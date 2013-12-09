INCLUDES = \
    action.h \
    cause.h \
    connections.h \
    name.h \
    flow.h \
    lib.h \
    poll.h \
    state.h \
    msg.h \
    worker.h

VPATH = ../libjoza

CSRC = action.c \
       cause.c \
	channel.c \
	diag.c \
	lib.c \
        log.c \
	main.c \
	packet.c \
	poll.c \
	raii.c \
	state.c \
	tput.c \
	joza_msg.c \
	msg.c \
	seq.c \
	iodir.c \
	worker.c

CPPSRC = 

OBJ := $(patsubst %.c,%.o,$(CSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

CPPFLAGS = \
    -I/usr/local/include \
    -Wall \
    -Wunused-macros \
    -Wendif-labels \
    -pedantic \
    `pkg-config glib-2.0 libczmq libzmq --cflags-only-I`

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
    `pkg-config glib-2.0 libczmq libzmq --cflags-only-other` \
    $(CWARN)

CC = gcc
# CC = /home/mike/studio/SolarisStudio12.3-linux-x86-bin/solarisstudio12.3/bin/cc
# CC = /usr/lib/clang-analyzer/scan-build/ccc-analyzer
# CC = clang

# compile the data but tell the compiler to separate the code into
# separate sections within the translation unit. This will be done for
# functions, classes, and external variables by using the following
# two compiler flags:

#ifeq (CC, gcc)
CFLAGS += -fdata-sections -ffunction-sections
#endif

LDFLAGS_LARGE = --verbose -Wl,-L/usr/local/lib
LDFLAGS_MEDIUM = --verbose -Wl,-L/usr/local/lib -Wl,--gc-sections -Wl,--print-gc-sections
LDFLAGS_SMALL = --verbose -Wl,-L/usr/local/lib  -Wl,--gc-sections -Wl,--print-gc-sections -Wl,--strip-all 

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
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS_MEDIUM) -lm \
	`pkg-config glib-2.0 libczmq libzmq --libs`

clean:
	-rm $(OBJ)
	-rm broker

.PHONY: check-syntax

check-syntax:
	$(CC) -std=gnu11 $(CPPFLAGS) $(CWARN) -pedantic -fsyntax-only $(CSRC)
	# $(CXX) -Wall -Wextra -pedantic -fsyntax-only $(CPPSRC)


