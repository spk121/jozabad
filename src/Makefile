SRCS = parch_state_engine.c
CC = clang
# CC = /home/mike/studio/SolarisStudio12.3-linux-x86-bin/solarisstudio12.3/bin/cc
# CC = /usr/lib/clang-analyzer/scan-build/ccc-analyzer
CFLAGS = -I../include -I/usr/local/include -Wall -Wextra -g -O0

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

parch_msg.c: parch_msg.xml
	./generate

test_node: parch_node.o test_node1.o parch_msg.o
	$(CC) -o $@ $^ $(LDFLAGS) -lczmq -lzmq

parch_broker: lib.o parch_broker.o parch_msg.o parch_state_engine.o
	$(CC) -o $@ $^ $(LDFLAGS) -lczmq -lzmq

clean:
	-rm test_node parch_broker *.o

.PHONY: check-syntax

check-syntax:
	$(CC) -Wall -Wextra -pedantic -fsyntax-only $(SRCS)

