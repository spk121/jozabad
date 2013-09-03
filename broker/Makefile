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

CSRC = pgetopt.c

CPPSRC = action.cpp \
    connections.cpp \
    dname.cpp \
    flow.cpp \
    lib.cpp \
    main.cpp \
    poll.cpp \
    state.cpp

OBJ := $(patsubst %.c,%.o,$(CSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

CPPFLAGS = \
    -I/usr/local/include \
    -Wall \
    -Wunused-macros \
    -Wendif-labels \
    -pedantic

CFLAGS = \
    -fstrict-aliasing \
    -fstrict-overflow \
    -ftree-vrp \
    -ggdb \
    -march=native \
    -O0

CCFLAGS = \
    -std=c11

CXXFLAGS = \
    -std=c++11

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

#    -fmudflapth \

CC = gcc
CXX = g++
# CC = /home/mike/studio/SolarisStudio12.3-linux-x86-bin/solarisstudio12.3/bin/cc
# CXX = /usr/lib/clang-analyzer/scan-build/ccc-analyzer
# CXX = clang
# CFLAGS = -std=c99  -Wall -Wextra -g -O0

# compile the data but tell the compiler to separate the code into
# separate sections within the translation unit. This will be done for
# functions, classes, and external variables by using the following
# two compiler flags:

ifeq (CXX, g++)
CXXFLAGS += -std=c++11
CXXFLAGS += -fdata-sections -ffunction-sections
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

action.o : action.cpp action.h
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN) -c -o $@ $<

connections.o : connections.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

diagnostic.o : diagnostic.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN) -c -o $@ $<

dname.o : dname.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

flow.o : flow.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

lib.o : lib.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

main.o : main.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

# Auto-generated file would have too many warnings if warnings were enabled.
msg.o : msg.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

packet.o : packet.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

pgetopt.o : pgetopt.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CCFLAGS) $(CWARN)  -c -o $@ $<

poll.o : poll.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

state.o : state.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

################################################3
# executable files


broker: $(OBJ) ../libjoza/libjoza.a
	$(CXX)  $(CPPFLAGS) $(CXXFLAGS) $(CWARN) -o $@ $^ $(LDFLAGS) -lczmq -lzmq

clean:
	-rm $(OBJ)
	-rm broker

.PHONY: check-syntax

check-syntax:
	$(CXX) -Wall -Wextra -pedantic -fsyntax-only $(CPPSRC)
	$(CC) -Wall -Wextra -pedantic -fsyntax-only $(CSRC)

