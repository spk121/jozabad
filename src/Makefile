INCLUDES = \
    ../include/action.h \
    ../include/channel.h \
    ../include/connections.h \
    ../include/diagnostic.h \
    ../include/direction.h \
    ../include/flow.h \
    ../include/lib.h \
    ../include/log.h \
    ../include/name.h \
    ../include/packet.h \
    ../include/poll.h \
    ../include/state.h \
    ../include/msg.h \
    ../include/throughput.h \
    ../include/window.h

SRC = action.cpp \
    connections.cpp \
    channel.cpp \
    flow.cpp \
    diagnostic.cpp \
    direction.cpp \
    lib.cpp \
    main.cpp \
    packet.cpp \
    poll.cpp \
    state.cpp \
    msg.cpp \
    throughput.cpp \
    window.cpp \

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
    -O2 \
    -std=c++11 \
    -Wall \
    -Warray-bounds \
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


CXX = g++
# CC = /home/mike/studio/SolarisStudio12.3-linux-x86-bin/solarisstudio12.3/bin/cc
# CXX = /usr/lib/clang-analyzer/scan-build/ccc-analyzer
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

msg.c: parch_msg.xml codec_c.gsl
../include/msg.h: parch_msg.xml codec_c.gsl
	./generate

################################################3
# header files

#../include/svc.h.gch : ../include/svc.h $(INCLUDES)
#	$(CXX) $(CXXFLAGS) $<

################################################3
# object files

action.o : action.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

channel.o : channel.cpp
	$(CXX) $(CXXFLAGS) -Wno-missing-field-initializers -c -o $@ $<

connections.o : connections.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

diagnostic.o : diagnostic.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

main.o : main.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

lib.o : lib.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

direction.o : direction.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

flow.o : flow.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Auto-generated file would have too many warnings if warnings were enabled.
msg.o : msg.c
	$(CXX) -c -o $@ $<

name.o : name.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

packet.o : packet.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

poll.o : poll.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

state.o : state.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

throughput.o : throughput.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

window.o : window.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

################################################3
# executable files


test_node: parch_node.o test_node1.o parch_msg.o
	$(CXX) -o $@ $^ $(LDFLAGS) -lczmq -lzmq

broker: $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS_MEDIUM) -lczmq -lzmq

clean:
	-rm ../include/svc.h.gch
	-rm $(OBJ) test_node1.o parch_node.o
	-rm test_node broker

.PHONY: check-syntax

check-syntax:
	$(CXX) -Wall -Wextra -pedantic -fsyntax-only $(SRCS) $(ISRCS)

