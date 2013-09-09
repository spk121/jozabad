#!/bin/bash

gcc -Wall -g -O0 \
    -std=c11 \
    -o t001_nonzero_first_ps.elf \
    t001_nonzero_first_ps.c \
    ../libjoza/joza_lib.c \
    ../libjoza/joza_msg.c \
    -lczmq -lzmq
