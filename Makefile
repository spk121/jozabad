INCLUDES = \
    include/action.h \
    include/channel.h \
    include/connections.h \
    include/diagnostic.h \
    include/direction.h \
    include/flow.h \
    include/lib.h \
    include/log.h \
    include/name.h \
    include/packet.h \
    include/poll.h \
    include/state.h \
    include/msg.h \
    include/throughput.h \
    include/window.h

SRC = src/action.cpp \
    src/connections.cpp \
    src/channel.cpp \
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

OBJ := $(patsubst %.cpp,%.o,$(SRC))

CPPFLAGS = \
    -I/usr/local/include \
    -Wall \
    -Wunused-macros \
    -Wendif-labels \
    -pedantic

CXXFLAGS = \
    $(CPPFLAGS) \
    -fstrict-aliasing \
    -fstrict-overflow \
    -ftree-vrp \
    -ggdb3 \
    -march=native \
    -O0 \
    -std=c++11 \
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

all: src/broker src/test_node
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
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/channel.o : src/channel.cpp
	$(CXX) $(CXXFLAGS) -Wno-missing-field-initializers -c -o $@ $<

src/connections.o : src/connections.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/diagnostic.o : src/diagnostic.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/main.o : src/main.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/lib.o : src/lib.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/direction.o : src/direction.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/flow.o : src/flow.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Auto-generated file would have too many warnings if warnings were enabled.
src/msg.o : src/msg.cpp
	$(CXX) -c -o $@ $<

src/name.o : src/name.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/packet.o : src/packet.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/poll.o : src/poll.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/state.o : src/state.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/throughput.o : src/throughput.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

src/window.o : src/window.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

################################################3
# executable files


src/test_node: src/parch_node.o src/test_node1.o src/msg.o
	$(CXX) -o $@ $^ $(LDFLAGS) -lczmq -lzmq

src/broker: $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS_MEDIUM) -lczmq -lzmq

clean:
	-rm include/svc.h.gch
	-rm $(OBJ) src/test_node1.o  src/parch_node.o
	-rm src/test_node src/broker

.PHONY: check-syntax

check-syntax:
	$(CXX) -Wall -Wextra -pedantic -fsyntax-only $(SRCS) $(ISRCS)

