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
    -O0 \
    -std=c11

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

CC = gcc
CXX = g++
# CC = /home/mike/studio/SolarisStudio12.3-linux-x86-bin/solarisstudio12.3/bin/cc
# CXX = /usr/lib/clang-analyzer/scan-build/ccc-analyzer
# CXX = clang
# CFLAGS = -std=c99  -Wall -Wextra -g -O0

ifeq (CC, gcc)
CXXFLAGS += -std=c11
CXXFLAGS += -fdata-sections -ffunction-sections
endif

LDFLAGS_LARGE = --verbose -Wl,-L/usr/local/lib
LDFLAGS_MEDIUM = -Wl,--gc-sections --verbose
LDFLAGS_SMALL = -Wl,--gc-sections -Wl,--strip-all --verbose

all: libjoza.a
#	tmp_broker_med tmp_broker_small

################################################3
# source files

joza_msg.c: joza.xml codec_c.gsl library.gsl
	gsl -script:codec_c joza.xml

joza_msg.h: joza.xml codec_c.gsl library.gsl
	gsl -script:codec_c joza.xml

################################################3
# header files

################################################3
# object files

joza_lib.o : joza_lib.c joza_lib.h joza_msg.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CWARN) -c -o $@ $<

joza_msg.o : joza_msg.c joza_msg.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CWARN) -c -o $@ $<

################################################3
# executable files


libjoza.a: joza_lib.o joza_msg.o
	ar rcs $@ $^

clean:
	-rm libjoza.a
	-rm joza_lib.o joza_msg.o
	-rm joza_msg.c joza_msg.h

.PHONY: check-syntax

check-syntax:
	$(CC) -Wall -Wextra -pedantic -fsyntax-only $(CRCS)

