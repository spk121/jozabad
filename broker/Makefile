INCLUDES = \
    action.h \
    connections.h \
    dname.h \
    flow.h \
    lib.h \
    log.h \
    pgetopt.h \
    poll.h \
    state.h \
    msg.h

VPATH = ../libjoza

CSRC = channel.c \
	lib.c \
	main.c \
	pgetopt.c \
	poll.c \
	tput.c \
	worker.c \
	joza_lib.c \
	joza_msg.c

CPPSRC = 

OBJ := $(patsubst %.c,%.o,$(CSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

CPPFLAGS = \
    -I/usr/local/include \
    -Wall \
    -Wunused-macros \
    -Wendif-labels \
    -pedantic

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
    -std=c11 \
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
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lczmq -lzmq -lm

clean:
	-rm $(OBJ)
	-rm broker

.PHONY: check-syntax

check-syntax:
	$(CXX) -Wall -Wextra -pedantic -fsyntax-only $(CPPSRC)
	$(CC) -Wall -Wextra -pedantic -fsyntax-only $(CSRC)

