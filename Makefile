INCLUDES = \
    include/action.h \
    include/connections.h \
    include/diagnostic.h \
    include/direction.h \
    include/flow.h \
    include/lib.h \
    include/log.h \
    include/name.h \
    include/packet.h \
    include/pgetopt.h \
    include/poll.h \
    include/state.h \
    include/msg.h \
    include/throughput.h \
    include/window.h

CSRC = src/pgetopt.c

CPPSRC = src/action.cpp \
    src/connections.cpp \
    src/flow.cpp \
    src/diagnostic.cpp \
    src/direction.cpp \
    src/lib.cpp \
    src/main.cpp \
    src/msg.cpp \
    src/name.cpp \
    src/packet.cpp \
    src/poll.cpp \
    src/state.cpp \
    src/throughput.cpp \
    src/window.cpp \

#ISRC := $(patsubst %.c,%.o,$(SRC))

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

all: src/broker src/w_echo src/w_flood
#	tmp_broker_med tmp_broker_small

################################################3
# source files

src/msg.cpp: src/parch_msg.xml src/codec_c.gsl
include/msg.h: src/parch_msg.xml src/codec_c.gsl
	./generate

################################################3
# header files

#include/svc.h.gch : include/svc.h $(INCLUDES)
#	$(CXX) $(CXXFLAGS) $<

################################################3
# object files

src/action.o : src/action.cpp include/action.h include/msg.h
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN) -c -o $@ $<

#src/channel.o : src/channel.cpp
#	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN) -c -o $@ $<

src/connections.o : src/connections.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/diagnostic.o : src/diagnostic.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN) -c -o $@ $<

src/direction.o : src/direction.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/flow.o : src/flow.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/lib.o : src/lib.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/main.o : src/main.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

# Auto-generated file would have too many warnings if warnings were enabled.
src/msg.o : src/msg.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/name.o : src/name.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/packet.o : src/packet.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

#
src/pgetopt.o : src/pgetopt.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CWARN)  -c -o $@ $<

src/poll.o : src/poll.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/state.o : src/state.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/throughput.o : src/throughput.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/window.o : src/window.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/w_echo.o : src/w_echo.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

src/w_flood.o : src/w_flood.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $(CWARN)  -c -o $@ $<

################################################3
# executable files


src/broker: $(OBJ)
	$(CXX)  $(CPPFLAGS) $(CXXFLAGS) $(CWARN) -o $@ $^ $(LDFLAGS) -lczmq -lzmq

src/w_echo: src/w_echo.o src/diagnostic.o src/direction.o src/msg.o src/packet.o src/throughput.o
	$(CXX)  $(CPPFLAGS) $(CXXFLAGS) $(CWARN) -o $@ $^ $(LDFLAGS) -lczmq -lzmq

src/w_flood: src/w_flood.o src/diagnostic.o src/direction.o src/msg.o src/packet.o src/throughput.o
	$(CXX)  $(CPPFLAGS) $(CXXFLAGS) $(CWARN) -o $@ $^ $(LDFLAGS) -lczmq -lzmq

clean:
	-rm include/svc.h.gch
	-rm $(OBJ) src/test_node1.o  src/parch_node.o
	-rm src/w_echo src/broker

.PHONY: check-syntax

check-syntax:
	$(CXX) -Wall -Wextra -pedantic -fsyntax-only $(SRCS) $(ISRCS)

